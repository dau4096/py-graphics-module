#include "includes.h"
#include "global.h"
#include "utils.h"
using namespace std;



void prepareOpenGL() {
    //Set up any requirements for the context.
}



namespace graphics {

void init(std::string name, std::pair<int, int> resolution, std::pair<int, int> version) {
    if (shared::window) {return; /* GLFW window already active */}


    //GLFW
    if (!glfwInit()) {
    	//GLFW is not initialised properly
        throw std::runtime_error("Failed to init GLFW");
    }
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, version.first);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, version.second);
    shared::window = glfwCreateWindow(resolution.first, resolution.second, name.c_str(), nullptr, nullptr);
    if (!shared::window) {
    	//GLFW could not create window.
        throw std::runtime_error("Failed to create window");
    }
    glfwMakeContextCurrent(shared::window);


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


int screenspaceShader(std::string filePath) {
    //For screenspace shaders. Obviously.
    if (!shared::init) {
        utils::cerr("You need to initialise GL first → gl.init()");
        return -1;
    }

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
    program.createProgram({vertex, fragment});

    int shaderID = shared::shaders.size();
    shared::shaders.emplace_back(program);
    return shaderID; //Shader reference for the python module.
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