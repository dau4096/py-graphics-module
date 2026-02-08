/* gl.cpp */
//Main file for the py "module".

#include "src/includes.h"
#include "src/constants.h"
#include "src/global.h"
#include "src/utils.h"
#include "src/graphics.h"

using namespace std;


namespace py = pybind11;



void verbose() {
	shared::verbose = true;
}


//// PYBIND11 STUFF ////
//The module name in py is "gl".
PYBIND11_MODULE(gl, m) {
	m.doc() = "OpenGL py abstraction module";

	//Config
	m.def("verbose", verbose); //gl.verbose() [Should the file give console output for actions taken?]




	//Values
	//OpenGL shader types
	m.attr("VERTEX_SHADER")   = GL_VERTEX_SHADER;
	m.attr("FRAGMENT_SHADER") = GL_FRAGMENT_SHADER;
	m.attr("COMPUTE_SHADER")  = GL_COMPUTE_SHADER;

	//gl Shader types
	py::enum_<ShaderType>(m, "ShaderType") //Shader Type Enum
		.value("ST_NONE", ShaderType::ST_NONE)
		.value("ST_WORLDSPACE", ShaderType::ST_WORLDSPACE)
		.value("ST_SCREENSPACE", ShaderType::ST_SCREENSPACE)
		.value("ST_COMPUTE", ShaderType::ST_COMPUTE)
		.export_values();




	//Manager Functions
	m.def("init", &graphics::init, //gl.init(name="", resolution=(1,1), version=(3,3))
		py::arg("name") = "",
		py::arg("resolution") = std::pair<int, int>{1, 1},
		py::arg("version") = std::pair<int, int>{3, 3}
	);
	m.def("terminate", &graphics::terminate); //gl.terminate()

	);


	//OpenGL abstractions
	m.def("load_shader", &graphics::loadShader, //gl.load_shader(type=ST_NONE, filePathA="", filePathB="");
		py::arg("type") = ST_NONE,
		py::arg("filePathA") = "",
		py::arg("filePathB") = ""
	);

	m.def("add_uniform_value", &graphics::addUniformValue, //gl.add_uniform_value(shader=0, name="", value=0.0);
		py::arg("shader") = 0,
		py::arg("name") = "",
		py::arg("value") = 0.0f
	);

}
//// PYBIND11 STUFF ////
