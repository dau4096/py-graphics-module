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


void updateWindow() {
	if (shared::window) {
		glfwSwapBuffers(shared::window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	else {utils::cerr("You need to initialise GL first → gl.init()");}
}

void pollEvents() {
	if (shared::window) {
		glfwPollEvents();
		previousKeyMap = currentKeyMap;
	}
	else {utils::cerr("You need to initialise GL first → gl.init()");}
}


bool keyHeld(int key) {
	return currentKeyMap[key];
}

bool keyPressed(int key) {
	return currentKeyMap[key] && !previousKeyMap[key];
}

bool keyReleased(int key) {
	return !currentKeyMap[key] && previousKeyMap[key];
}

bool windowOpen() {
	if (!shared::window) {return false; /* No window open so "close" it */}
	return !glfwWindowShouldClose(shared::window);
}

void requestClose() {
	if (shared::window) {
		glfwSetWindowShouldClose(shared::window, GLFW_TRUE);
	}
}




//// PYBIND11 STUFF ////
//The module name in py is "gl".
PYBIND11_MODULE(gl, m) {
	py::options opt;
	opt.show_user_defined_docstrings();

		m.doc() = R"doc(
OpenGL abstraction module.

Intended to simplify the OpenGL process to the higher-level steps such as;
- Load shaders
- Manage window
- Run shaders
- Poll inputs


Requires;
- OpenGL 3.3+
- GLFW
- GLEW
)doc";

	//Config
	m.def("verbose", verbose, "Configures whether the module can print to console."); //gl.verbose() [Should the file give console output for actions taken?]




	//Values
	//gl Shader types
	py::enum_<ShaderType>(m, R"doc(
ShaderType
----------
- ST_NONE        : No shader.
- ST_WORLDSPACE  : Shader on 3D geometry.
- ST_SCREENSPACE : Shader in screenspace, 2D.
- ST_COMPUTE     : Compute shader.
)doc") //Shader Type Enum
		.value("ST_NONE", 			ShaderType::ST_NONE)
		.value("ST_WORLDSPACE", 	ShaderType::ST_WORLDSPACE)
		.value("ST_SCREENSPACE", 	ShaderType::ST_SCREENSPACE)
		.value("ST_COMPUTE", 		ShaderType::ST_COMPUTE)
		.export_values();


	//gl VAO formats
	py::enum_<VAOFormat>(m, R"doc(
VAOFormat
---------
- VAO_EMPTY           : Empty VAO. No data.
- VAO_POS_ONLY        : Only 3D position attribute.
- VAO_POS_UV2D        : Position and 2D UV.
- VAO_POS_UV3D        : Position and 3D UV (Z used for texture index)
- VAO_POS_NORMAL      : Position and 3D Normal vector
- VAO_POS_UV2D_NORMAL : Position, 2D UV and 3D Normal vector.
- VAO_POS_UV3D_NORMAL : Position, 3D UV and 3D Normal vector. (Z used for texture index)
)doc") //VAO Format Enum
		.value("VAO_EMPTY", 			VAOFormat::VAO_EMPTY)
		.value("VAO_POS_ONLY", 			VAOFormat::VAO_POS_ONLY)
		.value("VAO_POS_UV2D", 			VAOFormat::VAO_POS_UV2D)
		.value("VAO_POS_UV3D", 			VAOFormat::VAO_POS_UV3D)
		.value("VAO_POS_NORMAL", 		VAOFormat::VAO_POS_NORMAL)
		.value("VAO_POS_UV2D_NORMAL", 	VAOFormat::VAO_POS_UV2D_NORMAL)
		.value("VAO_POS_UV3D_NORMAL", 	VAOFormat::VAO_POS_UV3D_NORMAL)
		.export_values();




	//Manager Functions
	m.def("init", &graphics::init, //gl.init(name="", resolution=(1,1), version=(3,3))
		py::arg("name")="GLFW/py-graphics", py::arg("resolution")=std::pair<int, int>{1, 1}, py::arg("version")=std::pair<int, int>{3, 3},
		R"doc(
Initialises the OpenGL context and its associated GLFW window, if required.

Parameters
----------
name : str, optional
	Window title.
resolution : tuple[int, int], optional
	Resolution of the window to open. If set to (1, 1) or below, window is automatically hidden.
version : tuple[int, int], optional
	OpenGL version Major|Minor. Assumed as CORE.

Raises
------
RuntimeError
	If GLFW fails to initialise or window fails to open.
)doc"
	);

	m.def("terminate", &graphics::terminate, //gl.terminate()
		R"doc(
Terminates the GLFW window, and cleans up OpenGL objects.
)doc"
	); 


	m.def("window_open", &windowOpen, //gl.window_open()
		R"doc(
Returns a boolean if the GLFW window is open or not.

Returns
-------
bool
	Is the window open.
)doc"
	);


	m.def("was_key_pressed", &keyPressed, //gl.was_key_pressed()
		py::arg("key"),
		R"doc(
If key has been pressed since the last gl.poll_events() call.

Parameters
----------
key : int
	The key to check. Enums follow the format KEY_[name] for [name]s like "W", "S", "LEFT_SHIFT", "ESCAPE" etc.

Returns
-------
bool
	If the key was pressed since the last gl.poll_events() call.
)doc"
	);
	m.def("was_key_released", &keyReleased, //gl.was_key_released()
		py::arg("key"),
		R"doc(
If key has been released since the last gl.poll_events() call.

Parameters
----------
key : int
	The key to check. Enums follow the format KEY_[name] for [name]s like "W", "S", "LEFT_SHIFT", "ESCAPE" etc.

Returns
-------
bool
	If the key was released since the last gl.poll_events() call.
)doc"
	);


	m.def("is_key_held", &keyHeld, //gl.is_key_held()
		py::arg("keyis_key_held"),
		R"doc(
If key has been held since the last gl.poll_events() call.

Parameters
----------
key : int
	The key to check. Enums follow the format KEY_[name] for [name]s like "W", "S", "LEFT_SHIFT", "ESCAPE" etc.

Returns
-------
bool
	If the key was held since the last gl.poll_events() call.
)doc"
	);






	//OpenGL abstractions
	m.def("load_shader", &graphics::loadShader, //gl.load_shader(type=ST_NONE, filePathA="", filePathB="");
		py::arg("type"), py::arg("filePathA"), py::arg("filePathB") = "",
		R"doc(
Creates a shader.
For types;
- ST_NONE
  = No files required
- ST_WORLDSPACE
  = vertex filepath, fragment filepath
- ST_SCREENSPACE
  = fragment filepath
- ST_COMPUTE
  = compute filepath

Parameters
----------
type : ShaderType
	The type of shader to create.
filePathA : str, optional
	The first file to load from. Optional.
filePathB : str, optional
	The second file to load from. Optional.

Returns
-------
int
	Index of the created shader.
)doc"
	);


	m.def("add_uniform_value", &graphics::addUniformValue, //gl.add_uniform_value(shader=-1, name="", value=0.0);
		py::arg("shader"), py::arg("name"), py::arg("value"),
		R"doc(
Adds a value to be passed into the shader.
The value is cached, so if it changes per-shader-run, this must be called every time.

Parameters
----------
shader : int
	The index of the shader to add uniform to.
name : str
	The name of the uniform to assign.
value : bool|int|float|_vec2|_vec3|_vec4
	Takes any 1-4D value that can be cast to a numerical value to bind at shader runtime.
)doc"
	);


	m.def(
		"add_vao",
		[](int shader, VAOFormat format, py::list values) {
			std::vector<float> vec = values.cast<std::vector<float>>(); //Converts py::list to std::vector<float>.
			graphics::addVAO(shader, format, vec);
		},
		py::arg("shader"), py::arg("format")=VAO_EMPTY, py::arg("values")=py::list(),
		R"doc(
Adds vertices to a 3D shader. Takes a shader index to assign to, a vertex data format (VAOFormat) and a list of float values.

Parameters
----------
shader : int
	Shader index to assign to.
format : VAOFormat
	The format of the data. See docs for VAOFormat for the formats.
values : list[float]
	A dataset of floating point values to be used in the shader's VAO.
)doc"
	);


	m.def("run", &graphics::runShader, //gl.run(shader=-1, dispatch=(0, 0, 0));
		py::arg("shader")=-1, py::arg("dispatch")=std::array<int, 3>{0, 0, 0},
		R"doc(
Runs a given shader. If a compute shader, takes a list of 3 elements as number of X/Y/Z threads to dispatch.

Parameters
----------
shader : int
	Shader index to assign to.
dispatch : list[int, int, int]
	Number of X/Y/Z threads to dispatch, only used if the shader is ST_COMPUTE type.
)doc"
	);


	m.def("update_window", &updateWindow, R"doc( Updates the window's framebuffer and other systems. )doc");


	m.def("poll_events", &pollEvents, R"doc( Check for GLFW inputs and events. )doc");


	m.def("configure", &graphics::configure,
		py::arg("type") = ST_NONE,
		R"doc(
Configures OpenGL settings for this type of shader pass.

Parameters
----------
type : ShaderType
	Type of shader to configure for. See (ShaderType) for info on types.
)doc"
	);





	//To easier passthrough GLFW keys.
	#define EXPORT_KEY(NAME) m.attr("KEY_" #NAME) = GLFW_KEY_##NAME;
	//GLFW key enums            //Continued
	EXPORT_KEY(ESCAPE); 		EXPORT_KEY(SPACE);
	EXPORT_KEY(LEFT_SHIFT);		EXPORT_KEY(RIGHT_SHIFT);
	EXPORT_KEY(LEFT_CONTROL);	EXPORT_KEY(RIGHT_CONTROL);
	EXPORT_KEY(LEFT_ALT);		EXPORT_KEY(RIGHT_ALT);

	EXPORT_KEY(1);				EXPORT_KEY(2);
	EXPORT_KEY(3); 				EXPORT_KEY(4);
	EXPORT_KEY(5);				EXPORT_KEY(6);
	EXPORT_KEY(7);				EXPORT_KEY(8);
	EXPORT_KEY(9);				EXPORT_KEY(0);

	EXPORT_KEY(Q);				EXPORT_KEY(W);
	EXPORT_KEY(E); 				EXPORT_KEY(R);
	EXPORT_KEY(T);				EXPORT_KEY(Y);
	EXPORT_KEY(U);				EXPORT_KEY(I);
	EXPORT_KEY(O);				EXPORT_KEY(P);

	EXPORT_KEY(A);				EXPORT_KEY(S);
	EXPORT_KEY(D);				EXPORT_KEY(F);
	EXPORT_KEY(G);				EXPORT_KEY(H);
	EXPORT_KEY(J);				EXPORT_KEY(K);
	EXPORT_KEY(L);
								EXPORT_KEY(Z);
	EXPORT_KEY(X);				EXPORT_KEY(C);
	EXPORT_KEY(V);				EXPORT_KEY(B);
	EXPORT_KEY(N);				EXPORT_KEY(M);

	EXPORT_KEY(ENTER);			EXPORT_KEY(BACKSPACE);
	EXPORT_KEY(TAB);			EXPORT_KEY(MINUS);
	EXPORT_KEY(EQUAL);			EXPORT_KEY(LEFT_BRACKET);
	EXPORT_KEY(RIGHT_BRACKET);	EXPORT_KEY(BACKSLASH);
	EXPORT_KEY(SEMICOLON);		EXPORT_KEY(APOSTROPHE);
	EXPORT_KEY(COMMA);			EXPORT_KEY(PERIOD);
	EXPORT_KEY(SLASH);
								EXPORT_KEY(UP);
	EXPORT_KEY(DOWN);			EXPORT_KEY(LEFT);
	EXPORT_KEY(RIGHT);
								EXPORT_KEY(F1);
	EXPORT_KEY(F2);				EXPORT_KEY(F3);
	EXPORT_KEY(F4);				EXPORT_KEY(F5);
	EXPORT_KEY(F6);				EXPORT_KEY(F7);
	EXPORT_KEY(F8);				EXPORT_KEY(F9);
	EXPORT_KEY(F10);			EXPORT_KEY(F11);
	EXPORT_KEY(F12);
}
//// PYBIND11 STUFF ////
