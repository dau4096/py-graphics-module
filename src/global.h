#pragma once
#include "includes.h"
#include "constants.h"
#include "utils.h"


enum UniformType {
	UV_INVAL, //Invalid
	UV_FLOAT, UV_INTEG, //1D values
	UV_FVEC2, UV_IVEC2, //2D values
	UV_FVEC3, UV_IVEC3, //3D values
	UV_FVEC4, UV_IVEC4, //4D values
};



namespace types {


struct UniformValue {
	UniformType type;
	glm::vec4 values;

	//Many overloads;
	UniformValue()             : type(UV_INVAL), values(0.0f, 0.0f, 0.0f, 0.0f) {}
	UniformValue(float v)      : type(UV_FLOAT), values(v, 0.0f, 0.0f, 0.0f) {}
	UniformValue(int v)        : type(UV_INTEG), values(static_cast<int>(v), 0.0f, 0.0f, 0.0f) {}
	UniformValue(bool v)       : type(UV_INTEG), values(((v) ? 1 : 0), 0.0f, 0.0f, 0.0f) {} //Bool also uses 1i.
	UniformValue(glm::vec2 v)  : type(UV_FVEC2), values(v, 0.0f, 0.0f) {}
	UniformValue(glm::ivec2 v) : type(UV_IVEC2), values(static_cast<glm::vec2>(v), 0.0f, 0.0f) {}
	UniformValue(glm::vec3 v)  : type(UV_FVEC3), values(v, 0.0f) {}
	UniformValue(glm::ivec3 v) : type(UV_IVEC3), values(static_cast<glm::ivec3>(v), 0.0f) {}
	UniformValue(glm::vec4 v)  : type(UV_FVEC4), values(v) {}
	UniformValue(glm::ivec4 v) : type(UV_IVEC4), values(static_cast<glm::ivec4>(v)) {}
};



//Singular part of a shader. Vertex, Fragment, Compute.
class ShaderObject {
public:
	GLuint GLindex = 0u;
	GLuint type; //GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER
	std::string source;
	std::string name;

	ShaderObject(GLuint shaderType, std::string& src, std::string name="")
		: type(shaderType), source(src), name(name) {}

	bool compile() {
		GLindex = glCreateShader(type);
		const char* srcC = source.c_str();
		glShaderSource(GLindex, 1, &srcC, nullptr);
		glCompileShader(GLindex);

		GLint status;
		glGetShaderiv(GLindex, GL_COMPILE_STATUS, &status);
		if (!status) {
			char buffer[512];
			glGetShaderInfoLog(GLindex, 512, nullptr, buffer);
			utils::cerr("Shader [", name, "] compile error: ", buffer);
			return false;
		}
		return true;
	}
};


class ShaderProgram {
private:
    GLuint _program = 0u; //OpenGL index
    bool _linked = false; //Ready to be used or not
    std::unordered_map<std::string, UniformValue> _uniforms; //Uniforms and their names to bind

    //Convert value to correct uniform type.
    inline GLint _convert(float value) {return static_cast<GLint>(value);}

public:
    ShaderProgram() = default;

    ~ShaderProgram() { //Delete instance callback
        if (_program) {glDeleteProgram(_program);}
    }

    bool createProgram(std::vector<ShaderObject> shaders) {
        _program = glCreateProgram();
        for (auto& sh : shaders) {
            if (!sh.compile()) {return false;}
            glAttachShader(_program, sh.GLindex);
        }

        glLinkProgram(_program);

        GLint status;
        glGetProgramiv(_program, GL_LINK_STATUS, &status);
        if (!status) {
            char buffer[512];
            glGetProgramInfoLog(_program, 512, nullptr, buffer);
            utils::cerr("Program link error: ", buffer);
            return false;
        }


        for (auto& sh : shaders) {
            glDetachShader(_program, sh.GLindex);
            glDeleteShader(sh.GLindex);
        }

        _linked = true;
        return true;
    }

    void use() {
        if (_linked) {glUseProgram(_program);}
    }

    GLuint programID() const {return _program;}

    template<typename T>
    void setUniform(const std::string& name, const T& val) {
        _uniforms[name] = UniformValue(val);
    }

    void applyUniforms() {
        for (const auto& [name, u] : _uniforms) {
            GLint loc = glGetUniformLocation(_program, name.c_str());
            if (loc == -1) {continue; /* Invalid location for this shader. */}

            switch (u.type) {
                case UniformType::UV_FLOAT: glUniform1f(loc,          u.values.x);  break;
                case UniformType::UV_INTEG: glUniform1i(loc, _convert(u.values.x)); break;
                case UniformType::UV_FVEC2: glUniform2f(loc,          u.values.x,           u.values.y);  break;
                case UniformType::UV_IVEC2: glUniform2i(loc, _convert(u.values.x), _convert(u.values.y)); break;
                case UniformType::UV_FVEC3: glUniform3f(loc,          u.values.x,           u.values.y,           u.values.z);  break;
                case UniformType::UV_IVEC3: glUniform3i(loc, _convert(u.values.x), _convert(u.values.y), _convert(u.values.z)); break;
                case UniformType::UV_FVEC4: glUniform4f(loc,          u.values.x,           u.values.y,           u.values.z,           u.values.w);  break;
                case UniformType::UV_IVEC4: glUniform4i(loc, _convert(u.values.x), _convert(u.values.y), _convert(u.values.z), _convert(u.values.w)); break;
                default: break;
            }
        }
    }
};




class Texture {
private:
	bool _valid = false;
	bool _image2D = false;
	bool _sampler2D = false;
	std::pair<GLint, GLint> _minMagFilters;
	std::pair<GLint, GLint> _wrap;

public:
	GLuint GLindex = 0;
	glm::ivec2 resolution = {0, 0};
	std::string filePath;

	Texture() = default;
	Texture(glm::ivec2 res) : _image2D(true), resolution(res) {}
	Texture(const std::string& path) : _sampler2D(true), filePath(path) {}

	bool loadTexture() {
		//TBA
		_valid = true;
		return _valid;
	}

	bool isValid() const {return _valid;}
};

}



namespace shared {

//Shared GL resources
inline GLFWwindow* window = nullptr;
inline std::vector<types::ShaderProgram> shaders;
inline std::vector<types::Texture> textures;

inline bool init = false;

}

