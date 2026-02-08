#pragma once
#include "includes.h"
#include "constants.h"



namespace structs {
	struct Shader {
		//TBA
	};

	struct Texture {
		//TBA
	};
}



namespace shared {

//Shared GL resources
inline GLFWwindow* window = nullptr;
inline std::vector<structs::Shader> shaders;
inline std::vector<structs::Texture> textures;

}

