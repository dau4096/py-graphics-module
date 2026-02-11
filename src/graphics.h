#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "includes.h"
#include "global.h"
#include "utils.h"


namespace graphics {

	void init(std::string name, glm::ivec2 resolution, glm::ivec2 version);

	int loadShader(ShaderType type, std::string filePathA, std::string filePathB);
	void configure(ShaderType type);
	bool addUniformValue(int shaderID, std::string uniformName, pybind11::object value);
	bool addVAO(int shaderID, VAOFormat format, std::vector<float> values);
	bool runShader(int shaderID, glm::uvec3 dispatchSize);

	void terminate();

}


#endif

