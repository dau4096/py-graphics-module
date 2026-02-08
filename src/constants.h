#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "includes.h"


enum UniformType {
	UV_INVAL, //Invalid
	UV_FLOAT, UV_INTEG, //1D values
	UV_FVEC2, UV_IVEC2, //2D values
	UV_FVEC3, UV_IVEC3, //3D values
	UV_FVEC4, UV_IVEC4, //4D values
};

enum ShaderType {
	ST_NONE,
	ST_WORLDSPACE,
	ST_SCREENSPACE,
	ST_COMPUTE
};


namespace shared {inline bool verbose = false; /* Should module give console output for actions taken? */}

namespace constants {

namespace display {
	//Access via constants::display::value
}

namespace misc {
	//Access via constants::misc::value
	constexpr int GL_ERROR_LENGTH = 1024;
}

}



#endif
