#include "includes.h"
#include "global.h"
#include "utils.h"
using namespace std;




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

	//Debug settings
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(openGLErrorCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
}


void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto it = currentKeyMap.find(key);
	if (it == currentKeyMap.end()) {return;}

	if (action == GLFW_PRESS) {it->second = true;} 
	else if (action == GLFW_RELEASE) {it->second = false;}
}


namespace compiler {

//Different types of shaders.
int computeShader(std::string filePath) {
	//For compute shaders.
	std::string compSource = utils::readFile(filePath);
	types::ShaderObject compute = types::ShaderObject(GL_COMPUTE_SHADER, compSource, utils::getFilename(filePath));
	
	types::ShaderProgram program;
	std::vector<types::ShaderObject> shaders = {compute,};

	int shaderID = shared::shaders.size();
	shared::shaders.emplace_back();
	shared::shaders.back().createProgram(shaders, ST_COMPUTE);
	return shaderID; //Shader reference for the python module.
}


int screenspaceShader(std::string filePath) {
	//For screenspace shaders.
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

	int shaderID = shared::shaders.size();
	shared::shaders.emplace_back();
	shared::shaders.back().createProgram(shaders, ST_SCREENSPACE);
	return shaderID; //Shader reference for the python module.
}


int worldspaceShader(std::string vertexFilePath, std::string fragmentFilePath) {
	//For shaders that draw onto triangles.
	std::string vertexSource = utils::readFile(vertexFilePath);
	types::ShaderObject vertex = types::ShaderObject(GL_VERTEX_SHADER, vertexSource, utils::getFilename(vertexFilePath));

	std::string fragSource = utils::readFile(fragmentFilePath);
	types::ShaderObject fragment = types::ShaderObject(GL_FRAGMENT_SHADER, fragSource, utils::getFilename(fragmentFilePath));
	
	types::ShaderProgram program;
	std::vector<types::ShaderObject> shaders = {vertex, fragment,};

	int shaderID = shared::shaders.size();
	shared::shaders.emplace_back();
	shared::shaders.back().createProgram(shaders, ST_WORLDSPACE);
	return shaderID; //Shader reference for the python module.
}

}



namespace config {

//Different types of shaders.
void computeShader() {
	//For compute shaders.
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glClearDepth(1.0f);
}


void screenspaceShader() {
	//For screenspace shaders.
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}


void worldspaceShader() {
	//For shaders that draw onto triangles.
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
}

}




bool inline shaderIDinRange(int shaderID) {return (shaderID < 0) || (shaderID >= static_cast<int>(shared::shaders.size()));}





namespace graphics {

void init(std::string name, std::pair<int, int> resolution, std::pair<int, int> version) {
	if (shared::window) {return; /* GLFW window already active */}


	//GLFW
	if (!glfwInit()) {
		//GLFW is not initialised properly
		utils::cerr("Failed to init GLFW");
	}
	utils::cout("[C++] Initialising OpenGL version [", version.first, version.second, "core ]");
	if ((resolution.first <= 1) || (resolution.second <= 1)) {
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, version.first);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, version.second);
	shared::window = glfwCreateWindow(resolution.first, resolution.second, name.c_str(), nullptr, nullptr);
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
	utils::cout("[C++] Successfully loaded GL");
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



bool addUniformValue(int shaderID, std::string uniformName, float value) {
	if (shaderIDinRange(shaderID)) {return false; /* Not in the shader list. */}
	//Sets or updates (if applicable) the shader's uniform value.
	shared::shaders[shaderID].setUniform(uniformName, value);
	return true;
}



void terminate() {
	//Told to close all active contexts and whatnot.
	if (shared::window) {
		glfwDestroyWindow(shared::window);
		shared::window = nullptr;
		glfwTerminate();
		shared::init = false;
		utils::cout("[C++] Successfully terminated GL");
	} else {
		utils::cout("[C++] Could not terminate: Was not initialised.");
	}
}

}