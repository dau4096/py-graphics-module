#include "includes.h"
#include "global.h"
#include "utils.h"


//////// PY MODULE ////////
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "typecast.h"
//////// PY MODULE ////////

using namespace std;
namespace py = pybind11;


namespace graphics {

void APIENTRY openGLErrorCallback(
		GLenum source,
		GLenum type, GLuint id,
		GLenum severity,
		GLsizei length, const GLchar* message,
		const void* userParam
	) {
	/*
	Nicely formatted callback from;
	[https://learnopengl.com/In-Practice/Debugging]
	*/
	if(id == 131169 || id == 131185 || id == 131218 || id == 131204) {return;}

	std::cout << "---------------" << std::endl << "Debug message (" << id << ") | " << message << std::endl;

	switch (source)
	{
		case GL_DEBUG_SOURCE_API:             {std::cout << "Source: API"; break;}
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   {std::cout << "Source: Window System"; break;}
		case GL_DEBUG_SOURCE_SHADER_COMPILER: {std::cout << "Source: Shader Compiler"; break;}
		case GL_DEBUG_SOURCE_THIRD_PARTY:     {std::cout << "Source: Third Party"; break;}
		case GL_DEBUG_SOURCE_APPLICATION:     {std::cout << "Source: Application"; break;}
		case GL_DEBUG_SOURCE_OTHER:           {std::cout << "Source: Other"; break;}
	} std::cout << std::endl;

	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:               {std::cout << "Type: Error"; break;}
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {std::cout << "Type: Deprecated Behaviour"; break;}
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  {std::cout << "Type: Undefined Behaviour"; break;} 
		case GL_DEBUG_TYPE_PORTABILITY:         {std::cout << "Type: Portability"; break;}
		case GL_DEBUG_TYPE_PERFORMANCE:         {std::cout << "Type: Performance"; break;}
		case GL_DEBUG_TYPE_MARKER:              {std::cout << "Type: Marker"; break;}
		case GL_DEBUG_TYPE_PUSH_GROUP:          {std::cout << "Type: Push Group"; break;}
		case GL_DEBUG_TYPE_POP_GROUP:           {std::cout << "Type: Pop Group"; break;}
		case GL_DEBUG_TYPE_OTHER:               {std::cout << "Type: Other"; break;}
	} std::cout << std::endl;
	
	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:         {std::cout << "Severity: high"; break;}
		case GL_DEBUG_SEVERITY_MEDIUM:       {std::cout << "Severity: medium"; break;}
		case GL_DEBUG_SEVERITY_LOW:          {std::cout << "Severity: low"; break;}
		case GL_DEBUG_SEVERITY_NOTIFICATION: {std::cout << "Severity: notification"; break;}
	} std::cout << std::endl;
	std::cout << std::endl;

	if (constants::misc::PAUSE_ON_OPENGL_ERROR) {
		utils::pause();
	}
}


void prepareOpenGL() {
	//Set up any requirements for the context.
	utils::cout("Configuring OpenGL-wide options");


	//Debug settings
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(openGLErrorCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

	glViewport(0, 0, shared::windowResolution.x, shared::windowResolution.y);

	//Create the empty VAO used in screenspace shaders
	glGenVertexArrays(1, &constants::display::emptyVAO); //Yes technically not constant, but it will _NEVER_ change after now.
}


void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto it = currentKeyMap.find(key);
	if (it == currentKeyMap.end()) {return;}

	if (action == GLFW_PRESS) {it->second = true;} 
	else if (action == GLFW_RELEASE) {it->second = false;}
}



bool castMat3(py::object value, glm::mat3 &out) {
	try { out = value.cast<glm::mat3>(); return true; } catch(...) {}
	try { out = value.cast<glm::mat<3,3,float,glm::defaultp>>(); return true; } catch(...) {}
	return false;
}

bool castMat4(py::object value, glm::mat4 &out) {
	try { out = value.cast<glm::mat4>(); return true; } catch(...) {}
	try { out = value.cast<glm::mat<4,4,float,glm::defaultp>>(); return true; } catch(...) {}
	return false;
}




namespace compiler {

//Different types of shaders.
int computeShader(std::string filePath) {
	//For compute shaders.
	utils::cout(std::format("Compiling Compute shader [COMP: \"{}\"]", filePath));
	std::string compSource = utils::readFile(filePath);
	types::ShaderObject compute = types::ShaderObject(GL_COMPUTE_SHADER, compSource, utils::getFilename(filePath));
	
	types::ShaderProgram program;
	std::vector<types::ShaderObject> shaders = {compute,};

	int shaderID = shared::numberOfShaders;
	shared::shaders[shared::numberOfShaders++].createProgram(shaders, ST_COMPUTE);
	return shaderID; //Shader reference for the python module.
}


int screenspaceShader(std::string filePath) {
	//For screenspace shaders.
	utils::cout(std::format("Compiling Screenspace shader [FRAG: \"{}\"]", filePath));
	std::string vertexSource = R"(
/* screenspace.vert */
#version 460 core

out vec2 fragUV;

const vec2 v[4] = {
	vec2(-1.0f, -1.0f),
	vec2( 1.0f, -1.0f),
	vec2(-1.0f,  1.0f),
	vec2( 1.0f,  1.0f)
};

void main() {
	gl_Position = vec4(v[gl_VertexID], 0.0f, 1.0f);
	fragUV = clamp(v[gl_VertexID], 0.0f, 1.0f);
}
)"; //This will _NEVER_ change. Keep hardcoded for that reason.
	types::ShaderObject vertex = types::ShaderObject(
		GL_VERTEX_SHADER, vertexSource, "screenspace.vert"
	);

	std::string fragSource = utils::readFile(filePath);
	types::ShaderObject fragment = types::ShaderObject(GL_FRAGMENT_SHADER, fragSource, utils::getFilename(filePath));
	
	types::ShaderProgram program;
	std::vector<types::ShaderObject> shaders = {vertex, fragment,};

	int shaderID = shared::numberOfShaders;
	shared::shaders[shared::numberOfShaders++].createProgram(shaders, ST_SCREENSPACE);
	return shaderID; //Shader reference for the python module.
}


int worldspaceShader(std::string vertexFilePath, std::string fragmentFilePath) {
	//For shaders that draw onto triangles.
	utils::cout(std::format("Compiling Worldpsace shader [VERT: \"{}\", FRAG: \"{}\"]", vertexFilePath, fragmentFilePath));
	std::string vertexSource = utils::readFile(vertexFilePath);
	types::ShaderObject vertex = types::ShaderObject(GL_VERTEX_SHADER, vertexSource, utils::getFilename(vertexFilePath));

	std::string fragSource = utils::readFile(fragmentFilePath);
	types::ShaderObject fragment = types::ShaderObject(GL_FRAGMENT_SHADER, fragSource, utils::getFilename(fragmentFilePath));
	
	types::ShaderProgram program;
	std::vector<types::ShaderObject> shaders = {vertex, fragment,};

	int shaderID = shared::numberOfShaders;
	shared::shaders[shared::numberOfShaders++].createProgram(shaders, ST_WORLDSPACE);
	return shaderID; //Shader reference for the python module.
}

}



namespace config {

//Different types of shaders.
void computeShader() {
	//For compute shaders.
	utils::cout("Configuring OpenGL for [ST_COMPUTE]");
	glDisable(GL_BLEND);
}


void screenspaceShader() {
	//For screenspace shaders, 2D.
	utils::cout("Configuring OpenGL for [ST_SCREENSPACE]");
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);

}


void worldspaceShader() {
	//For shaders that draw onto triangles, 3D.
	utils::cout("Configuring OpenGL for [ST_WORLDSPACE]");
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glClearDepth(1.0f);

	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);

}

}


inline bool IDnotInRange(int ID, size_t max) {return (ID < 0) || (ID >= static_cast<int>(max));}


namespace matrices {



glm::mat4 getPerspectiveMatrix(int cameraID) {
	types::Camera& camera = shared::cameras[cameraID];

	float aspectRatio = float(shared::windowResolution.x) / float(shared::windowResolution.y);
	float verticalFOV = 2 * atan(tan(camera.FOV * 0.5f) / aspectRatio);
	return glm::perspective(verticalFOV, aspectRatio, camera.nearZ, camera.farZ);
}

glm::mat4 getOrthographicMatrix() {
	return glm::ortho(0, shared::windowResolution.x, 0, shared::windowResolution.y);
}

glm::mat4 getViewMatrix(int cameraID) {
	types::Camera& camera = shared::cameras[cameraID];

	float sx = sin(camera.angle.x), cx = cos(camera.angle.x);
	float sy = sin(camera.angle.y), cy = cos(camera.angle.y);
	glm::vec3 forward = glm::vec3(
		sx*cy, cx*cy, sy
	);

	return glm::lookAt(camera.position, camera.position + forward, camera.up);
}

glm::mat4 getModelMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
	glm::mat4 translationMat = glm::mat4(
		1.0f, 	0.0f, 	0.0f, 	position.x,
		0.0f, 	1.0f, 	0.0f, 	position.y,
		0.0f, 	0.0f, 	1.0f, 	position.z,
		0.0f, 	0.0f, 	0.0f, 	1.0f
	);

	float sx = sin(rotation.x), cx = cos(rotation.x);
	float sy = sin(rotation.y), cy = cos(rotation.y);
	float sz = sin(rotation.z), cz = cos(rotation.z);
	glm::mat4 rotationMat = glm::mat4(
		cy*cz, 				cy*sz, 				-sy, 		0.0f,
		sx*sy*cz-cx*sz, 	sx*sy*sz+cx*cz, 	 sx*cy, 	0.0f,
		cx*sy*cz+sx*sz, 	cx*sy*sz-sx*cz, 	 cx*cy, 	0.0f,
		0.0f, 				0.0f, 				 0.0f, 		1.0f
	);

	glm::mat4 scaleMat = glm::mat4(
		scale.x,	0.0f, 		0.0f,		0.0f, 
		0.0f, 		scale.y,	0.0f, 		0.0f, 
		0.0f, 		0.0f, 		scale.z,	0.0f, 
		0.0f, 		0.0f, 		0.0f, 		1.0f
	);

	return rotationMat * scaleMat * translationMat;
}


//Manager func
glm::mat4 getMatrix(MatrixType type, int cameraID, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
	//Get matrix of type with data.
	//To be displayed if necessary. [OOR = Out-Of-Range]
	bool cameraID_OOR = IDnotInRange(cameraID, constants::misc::MAX_CAMERAS);
	std::string cameraID_OORmsg = std::format("Camera ID [{}] is invalid : Out of range [0 - {}]", cameraID, constants::misc::MAX_CAMERAS);

	switch (type) {
		case MAT_IDENTITY: {return glm::mat4(1.0f); /* Simple identity matrix. */}

		case MAT_PERSPECTIVE: {
			if (cameraID_OOR) {utils::cerr(cameraID_OORmsg);}
			return getPerspectiveMatrix(cameraID);
			break;
		}

		case MAT_ORTHOGRAPHIC: {
			return getOrthographicMatrix();
			break;
		}

		case MAT_VIEW: {
			if (cameraID_OOR) {utils::cerr(cameraID_OORmsg);}
			return getViewMatrix(cameraID);
			break;
		}

		case MAT_MODEL: {
			return getModelMatrix(position, rotation, scale);
			break;
		}

		default: {
			utils::cerr("Unknown matrix type. Must be valid MatrixType enum [MAT_PERSPECTIVE, MAT_ORTHOGRAPHIC, MAT_VIEW, MAT_MODEL]");
		}
	}

	return glm::mat4(0.0f);
}

}







namespace camera {


int assign(
	glm::vec3 pos, glm::vec3 rot, glm::vec3 up,
	float FOVdegrees, float FOVradians, float nz, float fz
) {
	if (shared::numberOfCameras >= constants::misc::MAX_CAMERAS) {
		utils::cerr(std::format("Maximum cameras exceeded : [{}]", constants::misc::MAX_CAMERAS));
	}
	if (glm::length(up) < 1e-5f) {
		utils::cerr(std::format("Camera [{}] up-direction cannot be (0, 0, 0).", shared::numberOfCameras));
	}

	bool useRad = FOVradians > 0.0f;
	if (!useRad && (FOVdegrees <= 0.0f)) {utils::cout(std::format(
		"Warning : Camera [{}] FOV is 0.", shared::numberOfCameras
	));}

	//Convert to radians, and clamp to range [-π → π | -180* → 180*]
	float fovFract = (useRad) ? (FOVradians / constants::maths::PI) : (FOVdegrees / 180.0f);
	float fov = glm::fract(abs(fovFract)) * constants::maths::PI;

	shared::cameras[shared::numberOfCameras].assign(pos, rot, up, fov, nz, fz); //Assign values to this entry.

	return shared::numberOfCameras++; //Auto-increments.
}


glm::vec3 getUp(int cameraID) {
	if (IDnotInRange(cameraID, constants::misc::MAX_CAMERAS)) {
		utils::cerr(std::format("Camera ID [{}] is invalid : Out of range [0 - {}]", cameraID, constants::misc::MAX_CAMERAS));
	}
	if (!(shared::cameras[cameraID].isValid())) {
		utils::cerr(std::format("Camera ID [{}] is invalid : Was never initialised, or was destroyed.", cameraID));
	}

	return shared::cameras[cameraID].up;
}


void setPosition(int cameraID, glm::vec3 position) {
	if (IDnotInRange(cameraID, constants::misc::MAX_CAMERAS)) {
		utils::cerr(std::format("Camera ID [{}] is invalid : Out of range [0 - {}]", cameraID, constants::misc::MAX_CAMERAS));
	}
	if (!(shared::cameras[cameraID].isValid())) {
		utils::cerr(std::format("Camera ID [{}] is invalid : Was never initialised, or was destroyed.", cameraID));
	}

	shared::cameras[cameraID].position = position;
}


void setAngle(int cameraID, glm::vec3 angle) {
	if (IDnotInRange(cameraID, constants::misc::MAX_CAMERAS)) {
		utils::cerr(std::format("Camera ID [{}] is invalid : Out of range [0 - {}]", cameraID, constants::misc::MAX_CAMERAS));
	}
	if (!(shared::cameras[cameraID].isValid())) {
		utils::cerr(std::format("Camera ID [{}] is invalid : Was never initialised, or was destroyed.", cameraID));
	}

	shared::cameras[cameraID].angle = angle;
}


void setFOV(int cameraID, float FOVdegrees, float FOVradians) {
	if (IDnotInRange(cameraID, constants::misc::MAX_CAMERAS)) {
		utils::cerr(std::format("Camera ID [{}] is invalid : Out of range [0 - {}]", cameraID, constants::misc::MAX_CAMERAS));
	}
	if (!(shared::cameras[cameraID].isValid())) {
		utils::cerr(std::format("Camera ID [{}] is invalid : Was never initialised, or was destroyed.", cameraID));
	}

	bool useRad = FOVradians > 0.0f;
	if (!useRad && (FOVdegrees <= 0.0f)) {utils::cout(std::format(
		"Warning : Camera [{}] FOV is 0.", cameraID
	));}

	//Convert to radians, and clamp to range [-π → π | -180* → 180*]
	float fovFract = (useRad) ? (FOVradians / constants::maths::PI) : (FOVdegrees / 180.0f);
	float fov = glm::fract(abs(fovFract)) * constants::maths::PI;

	shared::cameras[cameraID].FOV = fov;
}


void setZclip(int cameraID, float zNear, float zFar) {
	if (IDnotInRange(cameraID, constants::misc::MAX_CAMERAS)) {
		utils::cerr(std::format("Camera ID [{}] is invalid : Out of range [0 - {}]", cameraID, constants::misc::MAX_CAMERAS));
	}
	if (!(shared::cameras[cameraID].isValid())) {
		utils::cerr(std::format("Camera ID [{}] is invalid : Was never initialised, or was destroyed.", cameraID));
	}

	if (zNear > zFar) {
		utils::cerr(std::format("Near plane [{}] must be closer than the far plane [{}].", zNear, zFar));
	}

	shared::cameras[cameraID].nearZ = zNear;
	shared::cameras[cameraID].farZ = zFar;
}


void remove(int cameraID) {
	if (IDnotInRange(cameraID, constants::misc::MAX_CAMERAS)) {
		utils::cerr(std::format("Camera ID [{}] is invalid : Out of range [0 - {}]", cameraID, constants::misc::MAX_CAMERAS));
	}
	if (!(shared::cameras[cameraID].isValid())) {
		utils::cerr(std::format("Camera ID [{}] is invalid : Was never initialised, or was destroyed.", cameraID));
	}

	shared::cameras[cameraID].destroy(); //Set invalid and reset values to defaults.
}


}






void init(std::string name, glm::ivec2 resolution, const types::GLVersion& openGLVersion) {
	if (shared::window) {return; /* GLFW window already active */}
	utils::cout(std::format(
		"Initialising OpenGL version [{}.{}0 {}]",
		openGLVersion.major, openGLVersion.minor, ((openGLVersion.embedded) ? "ES" : "CORE")
	));
	if (!openGLVersion.valid()) {
		//Version is too old. Do not allow.
		utils::cerr(std::format(
			"OpenGL Version too old. Oldest supported versions are [3.30 CORE / 3.10 ES]. Attempted: [{}.{}0 {}]",
			openGLVersion.major, openGLVersion.minor, ((openGLVersion.embedded) ? "ES" : "CORE")
		));
		return;
	}
	shared::windowResolution = resolution;


	//GLFW
	if (!glfwInit()) {
		//GLFW is not initialised properly
		utils::cerr("Failed to init GLFW");
	}
	if ((resolution.x < 1) || (resolution.y < 1)) {
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	}
	openGLVersion.use();
	shared::window = glfwCreateWindow(resolution.x, resolution.y, name.c_str(), nullptr, nullptr);
	if (!shared::window) {
		//GLFW could not create window.
		utils::cerr("Failed to create window");
	}
	glfwMakeContextCurrent(shared::window);
	glfwSetKeyCallback(shared::window, glfwKeyCallback);


	//GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		utils::cerr("Failed to initialize GLEW");
		exit(1);
	}


	//OpenGL
	prepareOpenGL();



	shared::init = true;
	utils::cout("Successfully loaded GL-Module");
}




int loadShader(ShaderType type, std::string filePathA, std::string filePathB) {
	if (!shared::init) {
		utils::cerr("You need to initialise GL first → gl.init()");
		return -1;
	}

	switch (type) {
		case ST_COMPUTE:     {return compiler::computeShader(filePathA);}
		case ST_SCREENSPACE: {return compiler::screenspaceShader(filePathA);}
		case ST_WORLDSPACE:  {return compiler::worldspaceShader(filePathA, filePathB);}
		default:             {return -1; /* Invalid, unknown. */}
	}
	return -1; //Fallback
}


void configure(ShaderType type) {
	if (!shared::init) {
		utils::cerr("You need to initialise GL first → gl.init()");
		return;
	}

	switch (type) {
		case ST_COMPUTE:     {return config::computeShader();}
		case ST_SCREENSPACE: {return config::screenspaceShader();}
		case ST_WORLDSPACE:  {return config::worldspaceShader();}
		default:             {return; /* Invalid, unknown. */}
	}
	return; //Fallback
}



bool addUniformValue(int shaderID, std::string uniformName, py::object value) {
	if (IDnotInRange(shaderID, constants::misc::MAX_SHADERS)) {
		utils::cerr(std::format("Shader ID [{}] is invalid : Out of range [0 - {}]", shaderID, constants::misc::MAX_SHADERS));
	}

	types::ShaderProgram* shader = &(shared::shaders[shaderID]);
	utils::cout(std::format(
		"Added/Updated uniform \"{}\": [{}]",
		uniformName, //Name of the uniform being assigned
		std::string(py::str(py::type::of(value))) //Type of the uniform
	));
	utils::cout_inline(V_DEBUG, std::string(py::str(value))); //Value being assigned


	try {
		//1D values
		if (py::isinstance<py::int_>(value)) {shader->setUniform(uniformName, value.cast<int>()); return true;}
		if (py::isinstance<py::float_>(value)) {shader->setUniform(uniformName, value.cast<float>()); return true;}
		if (py::isinstance<py::bool_>(value)) {shader->setUniform(uniformName, value.cast<bool>()); return true;}

		//Vectors
		try {shader->setUniform(uniformName, value.cast<glm::vec2>()); return true;} catch(...) {}
		try {shader->setUniform(uniformName, value.cast<glm::vec3>()); return true;} catch(...) {}
		try {shader->setUniform(uniformName, value.cast<glm::vec4>()); return true;} catch(...) {}

		//Matrices
		glm::mat3 mat3; glm::mat4 mat4;
		if (castMat3(value, mat3)) {shader->setUniform(uniformName, mat3); return true;}
		if (castMat4(value, mat4)) {shader->setUniform(uniformName, mat4); return true;}


	} catch (const py::cast_error& e) {
		utils::cerr(std::format("Failed to cast uniform {}: {}", uniformName, e.what()));
		return false;
	}

	utils::cerr(std::format("Unsupported uniform type for '{}'", uniformName));
	return false;
}


bool addVAO(int shaderID, VAOFormat format, std::vector<float> values) {
	if (IDnotInRange(shaderID, constants::misc::MAX_SHADERS)) {
		utils::cerr(std::format("Shader ID [{}] is invalid : Out of range [0 - {}]", shaderID, constants::misc::MAX_SHADERS));
	}
	shared::shaders[shaderID].setVAO(format, values);
	return true;
}


bool runShader(int shaderID, glm::uvec3 dispatchSize) {
	if (IDnotInRange(shaderID, constants::misc::MAX_SHADERS)) {
		utils::cerr(std::format("Shader ID [{}] is invalid : Out of range [0 - {}]", shaderID, constants::misc::MAX_SHADERS));
	}
	types::ShaderProgram& shader = shared::shaders[shaderID];

	shader.use();
	shader.applyUniforms();
	return shader.run(dispatchSize, shaderID);
}



void terminate() {
	//Told to close all active contexts and whatnot.
	if (shared::window) {
		glfwDestroyWindow(shared::window);
		shared::window = nullptr;
		glfwTerminate();
		shared::init = false;

		shared::numberOfShaders = 0u;
		shared::numberOfTextures = 0u;

		utils::cout("Successfully terminated GL");
	} else {
		utils::cout("Could not terminate: Was not initialised.");
	}
}

}