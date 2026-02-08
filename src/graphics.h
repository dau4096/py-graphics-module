#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "includes.h"
#include "global.h"
#include "utils.h"


namespace graphics {

	void init(std::string name, std::pair<int, int> resolution, std::pair<int, int> version);

	int loadShader(ShaderType type, std::string filePathA, std::string filePathB);
	bool addUniformValue(int shaderID, std::string uniformName, float value);

	void terminate();

}


#endif

