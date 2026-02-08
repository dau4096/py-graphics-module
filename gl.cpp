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
//The name in py is "gl".
PYBIND11_MODULE(gl, m) {
    m.doc() = "OpenGL py abstraction module";

    m.def("verbose", verbose); //gl.verbose() [Should the file give console output for actions taken?]

    m.def("init", &graphics::init, //gl.init(name="", resolution=(1,1), version=(3,3))
          py::arg("name") = "",
          py::arg("resolution") = std::pair<int, int>{1, 1},
          py::arg("version") = std::pair<int, int>{3, 3});

    m.def("screenspace_shader", &graphics::screenspaceShader);

    m.def("terminate", &graphics::terminate); //gl.terminate()
}
//// PYBIND11 STUFF ////
