#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "includes.h"
#include "global.h"
#include "utils.h"


namespace graphics {

	void init(std::string name, std::pair<int, int> resolution, std::pair<int, int> version);

	int loadShader(ShaderType type, std::string filePathA, std::string filePathB);
	void configure(ShaderType type);
	bool addUniformValue(int shaderID, std::string uniformName, float value);
	bool addVAO(int shaderID, VAOFormat format, std::vector<float> values);
	bool runShader(int shaderID, std::array<int, 3> dispatchSize);

	void terminate();

}


#endif

