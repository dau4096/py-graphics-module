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
- Poll inputs


Requires;
- OpenGL 3.3+
- GLFW
- GLEW

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
		.value("ST_NONE", 			ShaderType::ST_NONE)
		.value("ST_WORLDSPACE", 	ShaderType::ST_WORLDSPACE)
		.value("ST_SCREENSPACE", 	ShaderType::ST_SCREENSPACE)
		.value("ST_COMPUTE", 		ShaderType::ST_COMPUTE)
		.export_values();


	//gl VAO formats
	py::enum_<VAOFormat>(m, "VAOFormat") //VAO Format Enum
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
		py::arg("name") = "",
		py::arg("resolution") = std::pair<int, int>{1, 1},
		py::arg("version") = std::pair<int, int>{3, 3},
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

	m.def("window_open", &windowOpen); //gl.window_open()

	m.def("was_key_pressed", &keyPressed, //gl.was_key_pressed()
		py::arg("key") = 0
	);
	m.def("was_key_released", &keyReleased, //gl.was_key_released()
		py::arg("key") = 0
	);
	m.def("is_key_held", &keyHeld, //gl.is_key_held()
		py::arg("keyis_key_held") = 0
	);


	//OpenGL abstractions
	m.def("load_shader", &graphics::loadShader, //gl.load_shader(type=ST_NONE, filePathA="", filePathB="");
		py::arg("type") = ST_NONE,
		py::arg("filePathA") = "",
		py::arg("filePathB") = ""
	);

	m.def("add_uniform_value", &graphics::addUniformValue, //gl.add_uniform_value(shader=-1, name="", value=0.0);
		py::arg("shader") = -1,
		py::arg("name") = "",
		py::arg("value") = 0.0f
	);

	m.def(
		"add_vao",
		[](int shader, VAOFormat format, py::list values) {
			std::vector<float> vec = values.cast<std::vector<float>>();
			graphics::addVAO(shader, format, vec);
		},
		py::arg("shader") = -1,
		py::arg("format") = VAO_EMPTY,
		py::arg("values") = py::list()
	);

	m.def("run", &graphics::runShader, //gl.run(shader=-1, dispatch=(0, 0, 0));
		py::arg("shader") = -1,
		py::arg("dispatch") = std::array<int, 3>{0, 0, 0}
	);

	m.def("update_window", &updateWindow);
	m.def("poll_events", &pollEvents);

	m.def("configure", &graphics::configure,
		py::arg("type") = ST_NONE
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
