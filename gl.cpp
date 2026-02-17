/* gl.cpp */
//Main file for the py "module".

#include "src/includes.h"
#include "src/doc.h"
#include "src/constants.h"
#include "src/global.h"
#include "src/utils.h"
#include "src/graphics.h"


//////// PY MODULE ////////
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "src/typecast.h"
//////// PY MODULE ////////


using namespace std;
namespace py = pybind11;



const std::unordered_map<Verbosity, std::string> vLevelMap = {
	{V_SILENT, "No output"}, {V_MINIMAL, "Minimal output"}, {V_DEBUG, "Full debugging output"}
};
void setVerbosity(Verbosity level) {
	shared::verbose = level;
	utils::cout(V_SILENT, std::format("Set console output level to: {}", vLevelMap.at(level)));
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
		//Keyboard/Mouse events
		glfwPollEvents();
		previousKeyMap = currentKeyMap;

		//Cursor movement.
		previousCursorPosition = currentCursorPosition;
		glfwGetCursorPos(shared::window, &currentCursorPosition.x, &currentCursorPosition.y);
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

glm::vec2 getCursorPos() {
	return glm::vec2(currentCursorPosition);
}

glm::vec2 getCursorDelta() {
	return glm::vec2(currentCursorPosition - previousCursorPosition);
}

void setCursorPos(glm::vec2 position) {
	//Defaults to 0, 0 if no arg passed.
	glfwSetCursorPos(shared::window, position.x, position.y);
}

void cursorShow() {glfwSetInputMode(shared::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);}
void cursorHide() {glfwSetInputMode(shared::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);}



void manageAddVAO(int shader, VAOFormat format, py::array_t<float> vertArr, py::array_t<int> indArray) {
	//Translate python array type (list, tuple, numpy.ndarray) into vector and pass to graphics::addVao() func.
	py::buffer_info vInfo = vertArr.request();
	float* vData = static_cast<float*>(vInfo.ptr);
	size_t vSize = vInfo.size;
	std::vector<float> vertices = std::vector<float>(vData, vData + vSize);

	py::buffer_info iInfo = indArray.request();
	int* iData = static_cast<int*>(iInfo.ptr);
	size_t iSize = iInfo.size;
	std::vector<int> indices = std::vector<int>(iData, iData + iSize);

	graphics::addVAO(shader, format, vertices, indices);
}


void manageInit(std::string name, glm::ivec2 resolution, glm::uvec2 versionUV3, bool core) {
	types::GLVersion version = types::GLVersion(versionUV3, !core); //Takes "Is embedded" but we have "Is core". They are opposites.
	graphics::init(name, resolution, version);
}














//// PYBIND11 STUFF ////
//The module name in py is "gl".
PYBIND11_MODULE(gl, m) {
	py::options opt;
	opt.show_user_defined_docstrings();
	m.doc() = documentation::meta::gl;




	//Values/Enums
	//Level of verbosity
	py::enum_<Verbosity>(m, documentation::GLenum::Verbosity) //Level of console output permitted
		.value("V_SILENT",  Verbosity::V_SILENT)
		.value("V_MINIMAL", Verbosity::V_MINIMAL)
		.value("V_DEBUG",   Verbosity::V_DEBUG)
		.export_values();


	//gl Shader types
	py::enum_<ShaderType>(m, documentation::GLenum::ShaderType) //Shader Type Enum
		.value("ST_NONE", 			ShaderType::ST_NONE)
		.value("ST_WORLDSPACE", 	ShaderType::ST_WORLDSPACE)
		.value("ST_SCREENSPACE", 	ShaderType::ST_SCREENSPACE)
		.value("ST_COMPUTE", 		ShaderType::ST_COMPUTE)
		.export_values();


	//gl VAO formats
	py::enum_<VAOFormat>(m, documentation::GLenum::VAOFormat) //VAO Format Enum
		.value("VAO_EMPTY", 			VAOFormat::VAO_EMPTY)
		.value("VAO_POS_ONLY", 			VAOFormat::VAO_POS_ONLY)
		.value("VAO_POS_UV2D", 			VAOFormat::VAO_POS_UV2D)
		.value("VAO_POS_UV3D", 			VAOFormat::VAO_POS_UV3D)
		.value("VAO_POS_NORMAL", 		VAOFormat::VAO_POS_NORMAL)
		.value("VAO_POS_UV2D_NORMAL", 	VAOFormat::VAO_POS_UV2D_NORMAL)
		.value("VAO_POS_UV3D_NORMAL", 	VAOFormat::VAO_POS_UV3D_NORMAL)
		.export_values();


	//Types of matrix that can be created
	py::enum_<MatrixType>(m, documentation::GLenum::MatrixType) //Matrix Type Enum
		.value("MAT_IDENTITY", 	MatrixType::MAT_IDENTITY)
		.value("MAT_PERSPECTIVE", 	MatrixType::MAT_PERSPECTIVE)
		.value("MAT_ORTHOGRAPHIC", 	MatrixType::MAT_ORTHOGRAPHIC)
		.value("MAT_VIEW", 			MatrixType::MAT_VIEW)
		.value("MAT_MODEL", 		MatrixType::MAT_MODEL)
		.export_values();


	//Types of matrix that can be created
	py::enum_<CameraDirection>(m, documentation::GLenum::CameraDirection) //Camera direction Enum
		.value("CD_FORWARD", 	CameraDirection::CD_FORWARD)
		.value("CD_RIGHT", 		CameraDirection::CD_RIGHT)
		.value("CD_UP", 		CameraDirection::CD_UP)
		.export_values();


	//Maximum quantities of certain types
	m.attr("MAX_SHADERS")  = constants::misc::MAX_SHADERS;
	m.attr("MAX_TEXTURES") = constants::misc::MAX_TEXTURES;
	m.attr("MAX_CAMERAS")  = constants::misc::MAX_CAMERAS;










	//Manager Functions
	m.def("verbose", setVerbosity, //gl.verbose() [Should the file give console output for actions taken?]
		py::arg("level")=V_SILENT, documentation::meta::verbose
	);

	m.def("init", &manageInit, //gl.init(name="", resolution=(0,0), version=(3,3), core=true)
		py::arg("name")="GLFW/py-graphics", py::arg("resolution")=glm::ivec2(0,0),
		py::arg("version")=glm::ivec2(3, 3), py::arg("core")=true,
		documentation::window::init
	);

	m.def("terminate", &graphics::terminate, //gl.terminate()
		documentation::window::terminate
	); 


	//GLFW events
	m.def("window_open", &windowOpen, //gl.window_open()
		documentation::window::windowOpen	
	);

	m.def("was_key_pressed", &keyPressed, //gl.was_key_pressed()
		py::arg("key"), documentation::window::wasKeyPressed
	);

	m.def("is_key_held", &keyHeld, //gl.is_key_held()
		py::arg("keyis_key_held"), documentation::window::isKeyHeld
	);

	m.def("was_key_released", &keyReleased, //gl.was_key_released()
		py::arg("key"), documentation::window::wasKeyReleased
	);

	m.def("update_window", &updateWindow,
		documentation::window::update
	);

	m.def("poll_events", &pollEvents,
		documentation::window::poll
	);

	m.def("get_cursor_position", &getCursorPos, //gl.get_cursor_position()
		documentation::window::getCursorPos
	);

	m.def("get_cursor_movement", &getCursorDelta, //gl.get_cursor_movement()
		documentation::window::cursorDelta
	);

	m.def("set_cursor_position", &setCursorPos, //gl.set_cursor_position(position=glm.vec2(0.0, 0.0));
		py::arg("position")=glm::vec2(0.0f, 0.0f),
		documentation::window::setCursorPos
	);

	m.def("cursor_show", &cursorShow, //gl.cursor_show();
		documentation::window::cursorShow
	);

	m.def("cursor_hide", &cursorHide, //gl.cursor_hide();
		documentation::window::cursorHide
	);






	//OpenGL abstractions
	m.def("load_shader", &graphics::loadShader, //gl.load_shader(type=ST_NONE, filePathA="", filePathB="");
		py::arg("type"), py::arg("filePathA"),
		py::arg("filePathB") = "", documentation::shader::load
	);


	m.def("add_uniform_value", &graphics::addUniformValue, //gl.add_uniform_value(shader=-1, name="", value=0.0);
		py::arg("shader"), py::arg("name"), py::arg("value"), documentation::shader::addUniform
	);


	m.def(
		"add_vao", &manageAddVAO,
		py::arg("shader"), py::arg("format")=VAO_EMPTY,
		py::arg("vertices")=py::list(), py::arg("indices")=py::list(),
		documentation::shader::addVAO
	);


	m.def("run", &graphics::runShader, //gl.run(shader=-1, dispatch=(0, 0, 0));
		py::arg("shader"), py::arg("dispatch")=glm::uvec3(0u, 0u, 0u), documentation::shader::run
	);


	m.def("configure", &graphics::configure,
		py::arg("type")=ST_NONE, py::arg("cull")=false,
		documentation::shader::configure
	);





	//Matrices
	m.def("get_matrix", &graphics::matrices::getMatrix, //gl.get_matrix(type=gl.MAT_IDENTITY, camera=-1, position=(0.0, 0.0, 0.0), rotation=(0.0, 0.0, 0.0), scale=(0.0, 0.0, 0.0));
		py::arg("type"), py::arg("camera")=-1,
		py::arg("position")=glm::vec3(0.0f, 0.0f, 0.0f),
		py::arg("rotation")=glm::vec3(0.0f, 0.0f, 0.0f),
		py::arg("scale")=glm::vec3(0.0f, 0.0f, 0.0f),
		documentation::matrix::get	
	);




	//Camera
	m.def("new_camera", &graphics::camera::assign, //gl.new_camera(position=(0, 0, 0), angle=(0, 0, 0), up=(0, 0, 1), fov_deg=0.0, fov_rad=0.0, near_z=-1.0, far_z=1.0);
		py::arg("position")=glm::vec3(0.0f, 0.0f, 0.0f), //Initial position
		py::arg("angle")=glm::vec3(0.0f, 0.0f, 0.0f), //Initial angle (Yaw, Pitch, Roll)
		py::arg("up")=glm::vec3(0.0f, 0.0f, 1.0f), //Up direction
		py::arg("fov_deg")=0.0f, py::arg("fov_rad")=0.0f, //FOV, Degrees/Radians (Radians favoured unless <= 0, then degrees.)
		py::arg("near_z")=-1.0f, py::arg("far_z")=1.0f, //Near/far plane distances.
		documentation::camera::newCamera	
	);


	m.def("get_camera_direction", &graphics::camera::getDirection, //gl.get_camera_direction(camera=-1, direction=gl.CD_FORWARD);
		py::arg("camera"), py::arg("direction"), documentation::camera::getDir
	);


	m.def("camera_set_new_position", &graphics::camera::setPosition, //gl.camera_set_new_position(camera=-1, position=(0, 0, 0));
		py::arg("camera"), py::arg("position"), documentation::camera::newPos
	);


	m.def("camera_set_new_angle", &graphics::camera::setAngle, //gl.camera_set_new_angle(camera=-1, angle=(0, 0, 0));
		py::arg("camera"), py::arg("angle"), documentation::camera::newAng
	);


	m.def("camera_set_new_fov", &graphics::camera::setFOV, //gl.camera_set_new_fov(camera=-1, fov_deg=0.0, fov_rad=0.0);
		py::arg("camera"), py::arg("fov_deg")=0.0f, py::arg("fov_rad")=0.0f,
		documentation::camera::newFOV
	);


	m.def("camera_set_new_clip", &graphics::camera::setZclip, //gl.camera_set_new_clip(camera=-1, z_near=0.0, z_far=0.0);
		py::arg("camera"), py::arg("z_near")=0.0f, py::arg("z_far")=0.0f,
		documentation::camera::newClip
	);

	m.def("camera_delete", &graphics::camera::remove, //gl.camera_delete(camera=-1);
		py::arg("camera"), documentation::camera::remove
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
