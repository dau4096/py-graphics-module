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


	//Functions
	m.def("init", &graphics::init, //gl.init(name="", resolution=(1,1), version=(3,3))
		py::arg("name") = "",
		py::arg("resolution") = std::pair<int, int>{1, 1},
		py::arg("version") = std::pair<int, int>{3, 3}
	);

	m.def("screenspace_shader", &graphics::screenspaceShader, //gl.screenspace_shader();
		py::arg("filePath") = ""
	);

	m.def("terminate", &graphics::terminate); //gl.terminate()


	//Values
	//OpenGL shader types
	m.attr("VERTEX_SHADER")   = GL_VERTEX_SHADER;
	m.attr("FRAGMENT_SHADER") = GL_FRAGMENT_SHADER;
	m.attr("COMPUTE_SHADER")  = GL_COMPUTE_SHADER;

	py::enum_<GraphicsMode>(m, "GraphicsMode") //Graphics mode Enum
		.value("GM_NONE", GraphicsMode::GM_NONE)
		.value("GM_WORLDSPACE", GraphicsMode::GM_WORLDSPACE)
		.value("GM_SCREENSPACE", GraphicsMode::GM_SCREENSPACE)
		.export_values();
}
//// PYBIND11 STUFF ////
