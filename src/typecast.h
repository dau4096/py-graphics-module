#ifndef TYPECAST_H
#define TYPECAST_H

#include "includes.h"
#include "constants.h"
namespace py = pybind11;


namespace pybind11 { namespace detail {

//Automatically convert lists to a GLM tvec if required.
template <glm::length_t L, typename T, glm::qualifier Q>
struct type_caster<glm::vec<L, T, Q>> {
public:
	using Vec = glm::vec<L, T, Q>;

	PYBIND11_TYPE_CASTER(Vec, _("glm_vec"));

	bool load(handle src, bool) {
		if (!py::isinstance<py::sequence>(src)) {return false;}

		py::sequence sequence = py::reinterpret_borrow<py::sequence>(src);
		if (sequence.size() != L) {return false; /* Wrong number of values */}
		for (glm::length_t i=0; i<L; i++) {value[i] = sequence[i].cast<T>();}

		return true;
	}

	static handle cast(const Vec& src, return_value_policy, handle) {
		py::tuple tuple = py::tuple(L);
		for (glm::length_t i=0; i<L; i++) {tuple[i] = src[i];}
		return tuple.release();
	}
};



//glm::mat4 ↔ glm.mat4 conversion
//    [C++]      [PY ]
template <glm::qualifier Q>
struct type_caster<glm::mat<4, 4, float, Q>> {
public:
	using Mat = glm::mat<4, 4, float, Q>;

	PYBIND11_TYPE_CASTER(Mat, _("glm_mat4"));

	bool load(handle src, bool) {
		if (!py::isinstance<py::sequence>(src))	{return false;}

		py::sequence sequence = py::reinterpret_borrow<py::sequence>(src);
		if (sequence.size() != 16) {return false; /* Wrong number of values. */}
		for (int i=0; i<16; i++) {value[i/4][i%4] = sequence[i].cast<float>();}

		return true;
	}

	static handle cast(const Mat& m, return_value_policy, handle) {
		py::tuple tuple = py::tuple(16);
		for (int i=0; i<16; i++) {tuple[i] = m[i/4][i%4];}
		return tuple.release();
	}
};



}}



#endif
