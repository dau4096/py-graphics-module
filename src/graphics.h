#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "includes.h"
#include "global.h"
#include "utils.h"


//////// PY MODULE ////////
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "typecast.h"
//////// PY MODULE ////////


namespace graphics {

	namespace matrices {

		glm::mat4 getMatrix(MatrixType type, int cameraID, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);

	}

	namespace camera {

		int assign(
			glm::vec3 pos, glm::vec3 rot, glm::vec3 up,
			float fov_deg, float fov_rad, float nz, float fz
		);

		glm::vec3 getDirection(int cameraID, CameraDirection cDir);
		void setPosition(int cameraID, glm::vec3 position);
		void setAngle(int cameraID, glm::vec3 angle);
		void setFOV(int cameraID, float FOVdegrees, float FOVradians);
		void setZclip(int cameraID, float zNear, float zFar);

		void remove(int cameraID);

	}


	void init(std::string name, glm::ivec2 resolution, const types::GLVersion& version);

	int loadShader(ShaderType type, std::string filePathA, std::string filePathB);
	void configure(ShaderType type, bool cull);
	bool addUniformValue(int shaderID, std::string uniformName, pybind11::object value);
	bool addVAO(int shaderID, VAOFormat format, std::vector<float> values, std::vector<int> indicesSigned);
	bool runShader(int shaderID, glm::uvec3 dispatchSize);

	void terminate();

}


#endif

