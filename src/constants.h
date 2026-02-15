#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "includes.h"


//Types of uniforms to accept
enum UniformType {
	UV_INVAL, //Invalid
	UV_FLOAT, UV_INTEG, //1D values
	UV_FVEC2, UV_IVEC2, //2D values
	UV_FVEC3, UV_IVEC3, //3D values
	UV_FVEC4, UV_IVEC4, //4D values
	UV_MAT33, UV_MAT44  //Matrices
};


//Types of shaders to load
enum ShaderType {
	ST_NONE,
	ST_WORLDSPACE,
	ST_SCREENSPACE,
	ST_COMPUTE,
};


//Formats for VAOs
enum VAOFormat {
	//2D → UV.xy
	//3D → UV.xy && textureID (example usage)
	VAO_EMPTY,                               //No values
	VAO_POS_ONLY,                            //Position
	VAO_POS_UV2D,        VAO_POS_UV3D,       //Position, UV
	VAO_POS_NORMAL,                          //Position, normal.
	VAO_POS_UV2D_NORMAL, VAO_POS_UV3D_NORMAL //Position, UV, normal.
};


//Matrix types to create
enum MatrixType {
	MAT_IDENTITY,
	MAT_PERSPECTIVE,
	MAT_ORTHOGRAPHIC,
	MAT_VIEW,
	MAT_MODEL,
};




inline std::unordered_map<int, bool> currentKeyMap = { //GLFW key enums and if they're pressed.
	{GLFW_KEY_ESCAPE, false},		{GLFW_KEY_SPACE, false},
	{GLFW_KEY_LEFT_SHIFT, false},	{GLFW_KEY_RIGHT_SHIFT, false},
	{GLFW_KEY_LEFT_CONTROL, false},	{GLFW_KEY_RIGHT_CONTROL, false},
	{GLFW_KEY_LEFT_ALT, false},		{GLFW_KEY_RIGHT_ALT, false},

	{GLFW_KEY_1, false},			{GLFW_KEY_2, false},
	{GLFW_KEY_3, false},			{GLFW_KEY_4, false},
	{GLFW_KEY_5, false},			{GLFW_KEY_6, false},
	{GLFW_KEY_7, false},			{GLFW_KEY_8, false},
	{GLFW_KEY_9, false},			{GLFW_KEY_0, false},

	{GLFW_KEY_Q, false}, 			{GLFW_KEY_W, false},
	{GLFW_KEY_E, false}, 			{GLFW_KEY_R, false},
	{GLFW_KEY_T, false}, 			{GLFW_KEY_Y, false},
	{GLFW_KEY_U, false}, 			{GLFW_KEY_I, false},
	{GLFW_KEY_O, false}, 			{GLFW_KEY_P, false},

	{GLFW_KEY_A, false},			{GLFW_KEY_S, false},
	{GLFW_KEY_D, false},			{GLFW_KEY_F, false},
	{GLFW_KEY_G, false},			{GLFW_KEY_H, false},
	{GLFW_KEY_J, false},			{GLFW_KEY_K, false},
	{GLFW_KEY_L, false},			
									{GLFW_KEY_Z, false},
	{GLFW_KEY_X, false},			{GLFW_KEY_C, false},
	{GLFW_KEY_V, false},			{GLFW_KEY_B, false},
	{GLFW_KEY_N, false},			{GLFW_KEY_M, false},

	{GLFW_KEY_ENTER, false},		{GLFW_KEY_BACKSPACE, false},
	{GLFW_KEY_TAB, false},			{GLFW_KEY_MINUS, false},
	{GLFW_KEY_EQUAL, false},		{GLFW_KEY_LEFT_BRACKET, false},
	{GLFW_KEY_RIGHT_BRACKET, false},{GLFW_KEY_BACKSLASH, false},
	{GLFW_KEY_SEMICOLON, false},	{GLFW_KEY_APOSTROPHE, false},
	{GLFW_KEY_COMMA, false},		{GLFW_KEY_PERIOD, false},
	{GLFW_KEY_SLASH, false},
									{GLFW_KEY_UP, false},
	{GLFW_KEY_DOWN, false},			{GLFW_KEY_LEFT, false},
	{GLFW_KEY_RIGHT, false},
									{GLFW_KEY_F1, false},
	{GLFW_KEY_F2, false},			{GLFW_KEY_F3, false},
	{GLFW_KEY_F4, false},			{GLFW_KEY_F5, false},
	{GLFW_KEY_F6, false},			{GLFW_KEY_F7, false},
	{GLFW_KEY_F8, false},			{GLFW_KEY_F9, false},
	{GLFW_KEY_F10, false},			{GLFW_KEY_F11, false},
	{GLFW_KEY_F12, false},
};
inline std::unordered_map<int, bool> previousKeyMap = currentKeyMap;



struct Attribute {
	GLint size;
};


namespace shared {inline bool verbose = false; /* Should module give console output for actions taken? */}

namespace constants {

	namespace maths {
		//Mathematical constants
		constexpr float PI = 3.141592f;
		constexpr float PI2 = PI * 2.0f;
		constexpr float EXP = 2.718281f;
		constexpr float INF = std::numeric_limits<float>::infinity();

		constexpr float TO_RAD = 0.017453f; //PI/180
		constexpr float TO_DEG = 57.29577f; //180/PI

	}

	namespace display {
		//Access via constants::display::value
		inline GLuint emptyVAO;

		static const std::unordered_map<VAOFormat, size_t> vertexFormatSizeMap = {
			{VAO_EMPTY, 0u}, {VAO_POS_ONLY, 3u}, {VAO_POS_NORMAL, 6u},
			{VAO_POS_UV2D, 5u}, {VAO_POS_UV3D, 6u},
			{VAO_POS_UV2D_NORMAL, 8u}, {VAO_POS_UV3D_NORMAL, 9u},
		};
		static const std::map<VAOFormat, std::vector<Attribute>> layouts = {
			{VAO_EMPTY,			 	{              }},
			{VAO_POS_ONLY,		 	{{3},          }},
			{VAO_POS_UV2D,       	{{3}, {2},     }},
			{VAO_POS_UV3D,       	{{3}, {3},     }},
			{VAO_POS_NORMAL,     	{{3}, {3},     }},
			{VAO_POS_UV2D_NORMAL,	{{3}, {2}, {3},}},
			{VAO_POS_UV3D_NORMAL,	{{3}, {3}, {3},}},
		};
	}

	namespace misc {
		//Access via constants::misc::value
		constexpr int GL_ERROR_LENGTH = 1024;
		constexpr bool PAUSE_ON_OPENGL_ERROR = true;

		constexpr size_t MAX_SHADERS  = 32u;
		constexpr size_t MAX_TEXTURES = 64u;
		constexpr size_t MAX_CAMERAS = 4u;
	}

}



#endif
