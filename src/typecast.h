#ifndef TYPECAST_H
#define TYPECAST_H

#include "includes.h"
#include "constants.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

namespace py = pybind11;
namespace pybind11::detail {


//Vector types
template <glm::length_t L, typename T, glm::qualifier Q>
struct type_caster<glm::vec<L, T, Q>> {
public:
	using Vec = glm::vec<L, T, Q>;
	using Scalar = T;

	PYBIND11_TYPE_CASTER(Vec, _<Vec>());

	bool load(py::handle src, bool) {
		if (!py::isinstance<py::sequence>(src)) {return false;}
		py::sequence seq = py::reinterpret_borrow<py::sequence>(src);
		if (seq.size() != L) {return false;}
		for (glm::length_t i=0; i<L; i++) {
			value[i] = seq[i].cast<T>();
		}
		return true;
	}

	static py::handle cast(const Vec& src, py::return_value_policy /* policy */, py::handle /* parent */) {
		py::tuple t(L);
		for (glm::length_t i=0; i<L; i++) {
			t[i] = src[i];
		}
		return t.release();
	}
};



//Matrix types
template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct type_caster<glm::mat<C, R, T, Q>> {
public:
	using Mat = glm::mat<C, R, T, Q>;

	PYBIND11_TYPE_CASTER(Mat, _("glm::mat"));

	bool load(py::handle src, bool) {
		//Pyglm types
		if (py::buffer::check_(src)) {
			py::buffer buf = py::reinterpret_borrow<py::buffer>(src);
			py::buffer_info info = buf.request();

			if (info.size != C * R) {return false;}

			T* data = static_cast<T*>(info.ptr);

			for (glm::length_t c=0; c<C; c++) {
				for (glm::length_t r=0; r<R; r++) {
					value[c][r] = data[c * R + r];
				}
			}

			return true;
		}

		//Lists
		if (py::isinstance<py::sequence>(src)) {
			py::sequence outer = py::reinterpret_borrow<py::sequence>(src);

			if (outer.size() != C) {return false;}

			for (glm::length_t c=0; c<C; c++) {
				py::sequence col = outer[c].cast<py::sequence>();
				if (col.size() != R) {return false;}

				for (glm::length_t r=0; r<R; r++) {
					value[c][r] = col[r].cast<T>();
				}
			}

			return true;
		}

		return false;
	}

	static py::handle cast(const Mat& src, py::return_value_policy /* policy */, py::handle /* parent */) {
		py::tuple outer(C);
		for (glm::length_t c=0; c<C; c++) {
			py::tuple col(R);
			for (glm::length_t r=0; r<R; r++) {
				col[r] = src[c][r];
			}
			outer[c] = col;
		}
		return outer.release();
	}
};


} //end the namespace "pybind11::detail"

#endif
