/* doc.h */
#ifndef DOC_H
#define DOC_H

#include <string>


//File that contains all of the docstrings for gl.cpp, to keep them contained more cleanly.
//Stored in their respective sections, as `inline constexpr const char*` types.

namespace documentation {


//Module management/meta docs
namespace meta {

//Main lib/module docstring
inline constexpr const char* gl = R"doc(
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


//How verbose the module should act (printing to cout)
inline constexpr const char* verbose = R"doc(
Configures whether the module can print to console.
)doc";

}



//Window/GLFW management
namespace window {

//Creating a new window
inline constexpr const char* init = R"doc(
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
)doc";


//Close this window.
inline constexpr const char* terminate = R"doc(
Terminates the GLFW window, and cleans up OpenGL objects.
)doc";


//Check if the GLFW window is still open
inline constexpr const char* windowOpen = R"doc(
Returns a boolean if the GLFW window is open or not.

Returns
-------
bool
	Is the window open.
)doc";


//Check if key was pressed
inline constexpr const char* wasKeyPressed = R"doc(
If key has been pressed since the last gl.poll_events() call.

Parameters
----------
key : int
	The key to check. Enums follow the format KEY_[name] for [name]s like "W", "S", "LEFT_SHIFT", "ESCAPE" etc.

Returns
-------
bool
	If the key was pressed since the last gl.poll_events() call.
)doc";


//Is key *being* pressed
inline constexpr const char* isKeyHeld = R"doc(
If key has been held since the last gl.poll_events() call.

Parameters
----------
key : int
	The key to check. Enums follow the format KEY_[name] for [name]s like "W", "S", "LEFT_SHIFT", "ESCAPE" etc.

Returns
-------
bool
	If the key was held since the last gl.poll_events() call.
)doc";


//Did it stop being pressed
inline constexpr const char* wasKeyReleased = R"doc(
If key has been released since the last gl.poll_events() call.

Parameters
----------
key : int
	The key to check. Enums follow the format KEY_[name] for [name]s like "W", "S", "LEFT_SHIFT", "ESCAPE" etc.

Returns
-------
bool
	If the key was released since the last gl.poll_events() call.
)doc";


//Update buffers etc
inline constexpr const char* update = R"doc(
Updates the window's framebuffer and other systems.
)doc";


//Poll events and keypresses
inline constexpr const char* poll = R"doc(
Check for GLFW inputs and events.
)doc";

}



//Enums
namespace GLenum {

//Level of console output permitted
inline constexpr const char* Verbosity = R"doc(
Verbosity
----------
- V_SILENT  : No console output from module.
- V_MINIMAL : Some console output - usually what stage was applied.
- V_DEBUG   : Used for debugging values and calls.
)doc";


//Types of shaders
inline constexpr const char* ShaderType = R"doc(
ShaderType
----------
- ST_NONE        : No shader.
- ST_WORLDSPACE  : Shader on 3D geometry.
- ST_SCREENSPACE : Shader in screenspace, 2D.
- ST_COMPUTE     : Compute shader.
)doc";


//Formats of VAO to accept
inline constexpr const char* VAOFormat = R"doc(
VAOFormat
---------
- VAO_EMPTY           : Empty VAO. No data.
- VAO_POS_ONLY        : Only 3D position attribute.
- VAO_POS_UV2D        : Position and 2D UV.
- VAO_POS_UV3D        : Position and 3D UV (Z used for texture index)
- VAO_POS_NORMAL      : Position and 3D Normal vector
- VAO_POS_UV2D_NORMAL : Position, 2D UV and 3D Normal vector.
- VAO_POS_UV3D_NORMAL : Position, 3D UV and 3D Normal vector. (Z used for texture index)
)doc";


//Types of matrix
inline constexpr const char* MatrixType = R"doc(
MatrixType
----------
- MAT_IDENTITY     : Identity matrix. Applies nothing when used.
- MAT_PERSPECTIVE  : Perspective matrix, Projection, Has distance scaling.
- MAT_ORTHOGRAPHIC : Orthographic matrix, Projection, No distance scaling.
- MAT_VIEW		   : View matrix, View, Camera translation and rotation.
- MAT_MODEL		   : Model matrix, Model, Vertex translation, rotation and scale.
)doc";


}



//Shader operations
namespace shader {

//Loading a shader from a file
inline constexpr const char* load = R"doc(
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
	The first file to load from. Only required for types other than ST_NONE.
filePathB : str, optional
	The second file to load from. Only required for ST_WORLDSPACE.
	
Raises
------
RuntimeError
	If gl.init() was not called previously.

Returns
-------
int
	Index of the created shader.
)doc";


//Adds a uniform value to this shader.
inline constexpr const char* addUniform = R"doc(
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
)doc";


//Adds a VAO to this shader.
inline constexpr const char* addVAO = R"doc(
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
)doc";


//Runs/dispatches this shader.
inline constexpr const char* run = R"doc(
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
)doc";


//Configuring OpenGL settings
inline constexpr const char* configure = R"doc(
Configures OpenGL settings for this type of shader pass.

Parameters
----------
type : ShaderType
	Type of shader to configure for. See (ShaderType) for info on types.
)doc";

}



//Matrix functions
namespace matrix {

//Gets/creates a matrix of given type.
inline constexpr const char* get = R"doc(
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
)doc";

}



//Camera functions
namespace camera {

//Assign new camera.
inline constexpr const char* newCamera = R"doc(
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
)doc";


inline constexpr const char* getUp = R"doc(
Returns the current up-direction of a camera from its ID.

Parameters
----------
camera : int
	The ID of the camera to access.

Returns
-------
vector[float, float float]
	The camera's up-direction.
)doc";


inline constexpr const char* newPos = R"doc(
Sets a new camera position (X/Y/Z | Up-direction depends on camera UP → Can be got via `gl.camera_up(camera=ID))`.) for a camera by its ID.

Parameters
----------
camera : int
	The ID of the camera to edit.
position : vector[float, float, float], optional
	3D Position to start the camera at.
)doc";


inline constexpr const char* newAng = R"doc(
Sets a new camera angle (YAW/PITCH/ROLL | X/Y/Z) for a camera by its ID.

Parameters
----------
camera : int
	The ID of the camera to edit.
angle : vector[float, float, float], optional
	Yaw/Pitch/Roll to start the camera at.
)doc";


inline constexpr const char* newFOV = R"doc(
Sets a new field-of-view angle (FOV) for a camera by its ID.

Parameters
----------
camera : int
	The ID of the camera to edit.
fov_deg : float
	Field of view, in degrees. Automatically converted to radians.
fov_rad : float
	Field of View, in radians. Preferred over fov_deg if both are present.
)doc";


inline constexpr const char* newClip = R"doc(
Sets a new z_near and z_far (clip-planes in 3D) for a camera by its ID.

Parameters
----------
camera : int
	The ID of the camera to edit.
z_near : float
	The distance of the near-plane from the camera. Negatives behind, Positive ahead, 0.0 at the camera. Must be lower than z_far.
z_far : float
	The distance of the far-plane from the camera. Negatives behind, Positive ahead, 0.0 at the camera. Must be higher than z_near.
)doc";


inline constexpr const char* remove = R"doc(
Deletes a camera from the set of active cameras. IDs used to refer to this camera will be made invalid.

Parameters
----------
camera : int
	The ID of the camera to delete.
)doc";

}




}


#endif
