/* gl.cpp */
//Main file for the py "module".

#include "src/includes.h"
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





void manageAddVAO(int shader, VAOFormat format, py::array_t<float> array) {
	//Translate python array type (list, tuple, numpy.ndarray) into vector and pass to graphics::addVao() func.
	py::buffer_info info = array.request();
	float* data = static_cast<float*>(info.ptr);
	size_t size = info.size;

	std::vector<float> vec = std::vector<float>(data, data + size);
	graphics::addVAO(shader, format, vec);
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

		m.doc() = R"doc(
OpenGL abstraction module.

Intended to simplify the OpenGL process to the higher-level steps such as;
- Load shaders
- Manage window
- Run shaders
- Poll inputs
Py-GLM module reccomended, but not required.
Any function docstrings that mention taking "vector[int int]" or similar can be assumed as either length-n tuples/lists, or Py-GLM types.


Requires;
- OpenGL [3.3 CORE+ / 3.1 ES+]
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


	//Types of matrix that can be created
	py::enum_<MatrixType>(m, R"doc(
MatrixType
----------
- MAT_IDENTITY     : Identity matrix. Applies nothing when used.
- MAT_PERSPECTIVE  : Perspective matrix, Projection, Has distance scaling.
- MAT_ORTHOGRAPHIC : Orthographic matrix, Projection, No distance scaling.
- MAT_VIEW		   : View matrix, View, Camera translation and rotation.
- MAT_MODEL		   : Model matrix, Model, Vertex translation, rotation and scale.
)doc") //Matrix Type Enum
		.value("MAT_IDENTITY", 	MatrixType::MAT_IDENTITY)
		.value("MAT_PERSPECTIVE", 	MatrixType::MAT_PERSPECTIVE)
		.value("MAT_ORTHOGRAPHIC", 	MatrixType::MAT_ORTHOGRAPHIC)
		.value("MAT_VIEW", 			MatrixType::MAT_VIEW)
		.value("MAT_MODEL", 		MatrixType::MAT_MODEL)
		.export_values();


	//Maximum quantities of certain types
	m.attr("MAX_SHADERS")  = constants::misc::MAX_SHADERS;
	m.attr("MAX_TEXTURES") = constants::misc::MAX_TEXTURES;
	m.attr("MAX_CAMERAS")  = constants::misc::MAX_CAMERAS;










	//Manager Functions
	m.def("init", &manageInit, //gl.init(name="", resolution=(0,0), version=(3,3), core=true)
		py::arg("name")="GLFW/py-graphics", py::arg("resolution")=glm::ivec2(0,0),
		py::arg("version")=glm::ivec2(3, 3), py::arg("core")=true,
		R"doc(
Initialises the OpenGL context and its associated GLFW window, if required.

Parameters
----------
name : str, optional
	Window title.
resolution : vector[int, int], optional
	Resolution of the window to open. If set to (1, 1) or below, window is automatically hidden.
version : vector[int, int], optional
	OpenGL version Major|Minor. Assumed as CORE.

Raises
------
RuntimeError
	If GLFW fails to initialise or window fails to open.
)doc");


	m.def("terminate", &graphics::terminate, //gl.terminate()
		R"doc(
Terminates the GLFW window, and cleans up OpenGL objects.
)doc"); 


	m.def("window_open", &windowOpen, //gl.window_open()
		R"doc(
Returns a boolean if the GLFW window is open or not.

Returns
-------
bool
	Is the window open.
)doc");


	m.def("was_key_pressed", &keyPressed, //gl.was_key_pressed()
		py::arg("key"), R"doc(
If key has been pressed since the last gl.poll_events() call.

Parameters
----------
key : int
	The key to check. Enums follow the format KEY_[name] for [name]s like "W", "S", "LEFT_SHIFT", "ESCAPE" etc.

Returns
-------
bool
	If the key was pressed since the last gl.poll_events() call.
)doc");


	m.def("was_key_released", &keyReleased, //gl.was_key_released()
		py::arg("key"), R"doc(
If key has been released since the last gl.poll_events() call.

Parameters
----------
key : int
	The key to check. Enums follow the format KEY_[name] for [name]s like "W", "S", "LEFT_SHIFT", "ESCAPE" etc.

Returns
-------
bool
	If the key was released since the last gl.poll_events() call.
)doc");


	m.def("is_key_held", &keyHeld, //gl.is_key_held()
		py::arg("keyis_key_held"), R"doc(
If key has been held since the last gl.poll_events() call.

Parameters
----------
key : int
	The key to check. Enums follow the format KEY_[name] for [name]s like "W", "S", "LEFT_SHIFT", "ESCAPE" etc.

Returns
-------
bool
	If the key was held since the last gl.poll_events() call.
)doc");






	//OpenGL abstractions
	m.def("load_shader", &graphics::loadShader, //gl.load_shader(type=ST_NONE, filePathA="", filePathB="");
		py::arg("type"), py::arg("filePathA"),
		py::arg("filePathB") = "", R"doc(
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
	
Raises
------
RuntimeError
	If gl.init() was not called previously.

Returns
-------
int
	Index of the created shader.
)doc");


	m.def("add_uniform_value", &graphics::addUniformValue, //gl.add_uniform_value(shader=-1, name="", value=0.0);
		py::arg("shader"), py::arg("name"), py::arg("value"), R"doc(
Adds a value to be passed into the shader.
The value is cached, so if it changes per-shader-run, this must be called every time.

Parameters
----------
shader : int
	The index of the shader to add uniform to.
name : str
	The name of the uniform to assign.
value : bool|int|float|_vec2|_vec3|_vec4|mat_
	Takes any 1-4D value or a [3x3/4x4] matrix to bind at shader runtime. Does not update per call - Must be re-added when it changes.
	
Raises
------
RuntimeError
	If this shader index is not valid.
)doc");


	m.def(
		"add_vao", &manageAddVAO,
		py::arg("shader"), py::arg("format")=VAO_EMPTY,
		py::arg("values")=py::list(), R"doc(
Adds vertices to a 3D shader. Takes a shader index to assign to, a vertex data format (VAOFormat) and a list of float values.

Parameters
----------
shader : int
	Shader index to assign to.
format : VAOFormat
	The format of the data. See docs for VAOFormat for the formats.
values : list[float]
	A dataset of floating point values to be used in the shader's VAO.

Raises
------
RuntimeError
	If this shader index is not valid.
)doc");


	m.def("run", &graphics::runShader, //gl.run(shader=-1, dispatch=(0, 0, 0));
		py::arg("shader"), py::arg("dispatch")=glm::uvec3(0u, 0u, 0u), R"doc(
Runs a given shader. If a compute shader, takes a list of 3 elements as number of X/Y/Z threads to dispatch.

Parameters
----------
shader : int
	Shader index to assign to.
dispatch : list[int, int, int], optional
	Number of X/Y/Z threads to dispatch, only used if the shader is ST_COMPUTE type.

Raises
------
RuntimeError
	If this shader index is not valid.
)doc");


	m.def("update_window", &updateWindow, R"doc( Updates the window's framebuffer and other systems. )doc");


	m.def("poll_events", &pollEvents, R"doc( Check for GLFW inputs and events. )doc");


	m.def("configure", &graphics::configure,
		py::arg("type") = ST_NONE, R"doc(
Configures OpenGL settings for this type of shader pass.

Parameters
----------
type : ShaderType
	Type of shader to configure for. See (ShaderType) for info on types.
)doc");





	//Matrices
	m.def("get_matrix", &graphics::matrices::getMatrix, //gl.get_matrix(type=gl.MAT_IDENTITY, camera=-1, position=(0.0, 0.0, 0.0), rotation=(0.0, 0.0, 0.0), scale=(0.0, 0.0, 0.0));
		py::arg("type"), py::arg("camera")=-1,
		py::arg("position")=glm::vec3(0.0f, 0.0f, 0.0f),
		py::arg("rotation")=glm::vec3(0.0f, 0.0f, 0.0f),
		py::arg("scale")=glm::vec3(0.0f, 0.0f, 0.0f),
		R"doc(
Creates a perspective projection matrix from the selected camera index.
Perspective matrices cause objects to get smaller with distance.

Parameters
----------
type : MatrixType
	Type of matrix to create: [MAT_IDENTITY, MAT_PERSPECTIVE, MAT_ORTHOGRAPHIC, MAT_VIEW, MAT_MODEL].
camera : int, optional
	Index of the camera to create the matrix from. Only required for [MAT_PERSPECTIVE, MAT_VIEW].
position : vector[float, float, float], optional
	Translation of the matrix. Only required for [MAT_MODEL].
rotation : vector[float, float, float], optional
	Rotation of the matrix around its origin. Only required for [MAT_MODEL].
scale : vector[float, float, float], optional
	Scale of the matrix relative to its origin. Only required for [MAT_MODEL].

Raises
------
RuntimeError
	For unknown type.

Returns
-------
tuple[tuple[float]]
	A 4x4 2D matrix of tuples representing a matrix. Can be converted to a py-glm mat4 directly.
)doc");




	//Camera
	m.def("new_camera", &graphics::camera::assign, //gl.new_camera(position=(0, 0, 0), angle=(0, 0, 0), up=(0, 0, 1), fov_deg=0.0, fov_rad=0.0, near_z=-1.0, far_z=1.0);
		py::arg("position")=glm::vec3(0.0f, 0.0f, 0.0f), //Initial position
		py::arg("angle")=glm::vec3(0.0f, 0.0f, 0.0f), //Initial angle (Yaw, Pitch, Roll)
		py::arg("up")=glm::vec3(0.0f, 0.0f, 1.0f), //Up direction
		py::arg("fov_deg")=0.0f, py::arg("fov_rad")=0.0f, //FOV, Degrees/Radians (Radians favoured unless <= 0, then degrees.)
		py::arg("near_z")=-1.0f, py::arg("far_z")=1.0f, //Near/far plane distances.
		R"doc(
Creates a new camera, returning the ID of this new camera.

Parameters
----------
position : vector[float, float, float], optional
	3D Position to start the camera at.
angle : vector[float, float, float], optional
	Yaw/Pitch/Roll to start the camera at.
up : vector[float, float, float], optional
	Up-direction of the camera. By default, +Z upwards.
fov_deg : float
	Field of view, in degrees. Automatically converted to radians.
fov_rad : float
	Field of View, in radians. Preferred over fov_deg if both are present.
near_z : float
	The distance from the camera (World units) the closest objects rendered are allowed to be.
far_z : float
	The distance from the camera (World units) the furthest objects rendered are allowed to be.

Raises
------
RuntimeError
	If there are already too many cameras created, or if the up-direction is invalid.

Returns
-------
int
	Index of the new camera. Used when modifying its data later, or when using it to create matrices.
)doc");


	m.def("camera_up", &graphics::camera::getUp, //gl.camera_up(camera=-1);
		py::arg("camera"),
		R"doc(
Returns the current up-direction of a camera from its ID.

Parameters
----------
camera : int
	The ID of the camera to access.

Returns
-------
vector[float, float float]
	The camera's up-direction.
)doc");


	m.def("camera_set_new_position", &graphics::camera::setPosition, //gl.camera_set_new_position(camera=-1, position=(0, 0, 0));
		py::arg("camera"), py::arg("position"),
		R"doc(
Sets a new camera position (X/Y/Z | Up-direction depends on camera UP → Can be got via `gl.camera_up(camera=ID))`.) for a camera by its ID.

Parameters
----------
camera : int
	The ID of the camera to edit.
position : vector[float, float, float], optional
	3D Position to start the camera at.
)doc");


	m.def("camera_set_new_angle", &graphics::camera::setAngle, //gl.camera_set_new_angle(camera=-1, angle=(0, 0, 0));
		py::arg("camera"), py::arg("angle"),
		R"doc(
Sets a new camera angle (YAW/PITCH/ROLL | X/Y/Z) for a camera by its ID.

Parameters
----------
camera : int
	The ID of the camera to edit.
angle : vector[float, float, float], optional
	Yaw/Pitch/Roll to start the camera at.
)doc");


	m.def("camera_set_new_fov", &graphics::camera::setFOV, //gl.camera_set_new_fov(camera=-1, fov_deg=0.0, fov_rad=0.0);
		py::arg("camera"), py::arg("fov_deg")=0.0f, py::arg("fov_rad")=0.0f,
		R"doc(
Sets a new field-of-view angle (FOV) for a camera by its ID.

Parameters
----------
camera : int
	The ID of the camera to edit.
fov_deg : float
	Field of view, in degrees. Automatically converted to radians.
fov_rad : float
	Field of View, in radians. Preferred over fov_deg if both are present.
)doc");


	m.def("camera_set_new_clip", &graphics::camera::setZclip, //gl.camera_set_new_clip(camera=-1, z_near=0.0, z_far=0.0);
		py::arg("camera"), py::arg("z_near")=0.0f, py::arg("z_far")=0.0f,
		R"doc(
Sets a new z_near and z_far (clip-planes in 3D) for a camera by its ID.

Parameters
----------
camera : int
	The ID of the camera to edit.
z_near : float
	The distance of the near-plane from the camera. Negatives behind, Positive ahead, 0.0 at the camera. Must be lower than z_far.
z_far : float
	The distance of the far-plane from the camera. Negatives behind, Positive ahead, 0.0 at the camera. Must be higher than z_near.
)doc");


	m.def("camera_delete", &graphics::camera::remove, //gl.camera_delete(camera=-1);
		py::arg("camera"),
		R"doc(
Deletes a camera from the set of active cameras. IDs used to refer to this camera will be made invalid.

Parameters
----------
camera : int
	The ID of the camera to delete.
)doc");








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
