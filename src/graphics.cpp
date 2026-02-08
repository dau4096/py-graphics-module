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

    prepareOpenGL();
    std::cout << "[C++] Successfully loaded GL" << std::endl;
}

void terminate() {
	//Told to close all active contexts and whatnot.
    if (shared::window) {
        glfwDestroyWindow(shared::window);
        shared::window = nullptr;
        glfwTerminate();
        std::cout << "[C++] Successfully terminated GL" << std::endl;
    } else {
        std::cout << "[C++] Could not terminate: Was not initialised." << std::endl;
    }
}

}