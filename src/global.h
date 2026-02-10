#pragma once
#include "includes.h"
#include "constants.h"
#include "utils.h"



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



//Contains data related to calling a shader.
//ST_COMPUTE     → localSize
//ST_WORLDSPACE  → VAO & index count
//ST_SCREENSPACE → N/A
struct ShaderCall {
	glm::uvec3 localSize;
	GLuint VAO;
	unsigned int numberOfVertices;

	ShaderCall() : localSize(0u, 0u, 0u), VAO(0), numberOfVertices(0u) {}
	ShaderCall(glm::uvec3& ls, GLuint vao, unsigned int nv)
		: localSize(ls), VAO(vao), numberOfVertices(nv) {}
};



//Singular part of a shader. Vertex, Fragment, Compute.
class ShaderObject {
public:
	GLuint GLindex = 0u;
	GLuint type = 0u; //GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER
	std::string source = "";
	std::string name = "";


	ShaderObject(GLuint shaderType, std::string& src, std::string name="")
		: type(shaderType), source(src), name(name) {}

	void destroy() {
		//Deletion
		if (GLindex > 0u) {
			glDeleteShader(GLindex);
			type = 0u;
			source = "";
			name = "";
		}
	}

	~ShaderObject() {destroy();}

	bool compile() {
		GLindex = glCreateShader(type);
		const char* srcC = source.c_str();
		glShaderSource(GLindex, 1, &srcC, nullptr);
		glCompileShader(GLindex);

		GLint status;
		glGetShaderiv(GLindex, GL_COMPILE_STATUS, &status);
		if (!status) {
			char buffer[constants::misc::GL_ERROR_LENGTH];
			glGetShaderInfoLog(GLindex, constants::misc::GL_ERROR_LENGTH, nullptr, buffer);
			utils::cout("\nShader [", name, "] compile error:");
			utils::cerr(buffer);
			return false;
		}
		if (GLEW_KHR_debug || GLEW_VERSION_4_3) {
			//Label it for debugging.
			glObjectLabel(GL_SHADER, GLindex, -1, name.c_str());
		}

		return true;
	}
};


//Full shader program.
class ShaderProgram {
private:
	GLuint _program = 0u; //OpenGL index
	bool _linked = false; //Ready to be used or not
	std::unordered_map<std::string, UniformValue> _uniforms; //Uniforms and their names to bind
	ShaderCall _call; //Contains data to be used when doing shader.run();

	//Convert value to correct uniform type.
	inline GLint _convert(float value) {return static_cast<GLint>(value);}

public:
	ShaderType type = ST_NONE; //Type of shader.


	//Default creation
	ShaderProgram() = default;


	//Stop shallow copying, delete the methods.
	ShaderProgram(const ShaderProgram&) = delete;
	ShaderProgram& operator=(const ShaderProgram&) = delete;


	//Move constructor
	ShaderProgram(ShaderProgram&& other) noexcept {
		_program = other._program;
		_linked = other._linked;
		type = other.type;
		_uniforms = std::move(other._uniforms);
		other._program = 0;
	}


	//Move operator
	ShaderProgram& operator=(ShaderProgram&& other) noexcept {
		if (this != &other) {
			if (_program) glDeleteProgram(_program);
			_program = other._program;
			_linked = other._linked;
			type = other.type;
			_uniforms = std::move(other._uniforms);
			other._program = 0;
		}
		return *this;
	}


	//Deletion
	~ShaderProgram() {
		if (_program) glDeleteProgram(_program);
	}


	bool createProgram(std::vector<ShaderObject>& shaders, ShaderType type) {
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
		if (type == ST_COMPUTE) { //Copy local size from compute shader def.
			//Find dispatch size def
			GLint workGroupSize[3];
			glGetProgramiv(_program, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);

			_call.localSize = glm::uvec3(
				static_cast<unsigned int>(workGroupSize[0]),
				static_cast<unsigned int>(workGroupSize[1]),
				static_cast<unsigned int>(workGroupSize[2])
			);
		}


		for (auto& sh : shaders) {
			//Delete shader, it has been used.
			sh.destroy();
		}

		_linked = true;
		this->type = type;
		return true;
	}


	void setVAO(VAOFormat format, std::vector<float>& values) {
		//Create VAO with given format and values.
		if (format == VAO_EMPTY) {
			_call.numberOfVertices = 0u;
			_call.VAO = constants::display::emptyVAO;
			return;
		}
		size_t vertexSizeSingular = constants::display::vertexFormatSizeMap.at(format);
		_call.numberOfVertices = values.size() / vertexSizeSingular;

	
		glGenVertexArrays(1, &(_call.VAO));
		glBindVertexArray(_call.VAO);

		GLuint VBO, EBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * values.size(), nullptr, GL_DYNAMIC_DRAW); //Reserve space

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _call.numberOfVertices, nullptr, GL_DYNAMIC_DRAW); //Reserve space


		size_t offset = 0u;
		unsigned int attribID = 0;
		//Loop through and set attributes based on the format. For instance, VAO_POS_UV2D_NORMAL = {3, 2, 3}.
		for (const Attribute& attr : (constants::display::layouts.at(format))) {
			glVertexAttribPointer(attribID, attr.size, GL_FLOAT, GL_FALSE, vertexSizeSingular * sizeof(float), (void*)(offset * sizeof(float)));
			glEnableVertexAttribArray(attribID++);
			offset += attr.size;
		}

		glBindVertexArray(0);
	}


	inline void use() {if (_linked) {glUseProgram(_program);} else {utils::cerr("Must create shader first, before using it.");}}


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


	bool run(glm::uvec3 dispatchSize) {
		switch (this->type) {
			case ST_COMPUTE: {
				//Dispatch compute
				glm::uvec3 dispatchGroups = (dispatchSize + _call.localSize - 1u) / _call.localSize;
				glDispatchCompute(dispatchGroups.x, dispatchGroups.y, dispatchGroups.z);
				break;
			}
			case ST_WORLDSPACE: {
				//Run worldspace (3D)
				glBindVertexArray(_call.VAO);
				glDrawElements(GL_TRIANGLES, _call.numberOfVertices, GL_UNSIGNED_INT, nullptr);
				glBindVertexArray(0);
				break;
			}
			case ST_SCREENSPACE: {
				//Run screenspace (2D)
				glBindVertexArray(constants::display::emptyVAO);
				glDrawArrays(GL_TRIANGLE_STRIP, 0u, 4u);
				glBindVertexArray(0);
				break;
			}
			default: {return false; /* Unknown type */}
		}
		return true;
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

inline size_t numberOfShaders;
inline size_t numberOfTextures;
inline std::array<types::ShaderProgram, constants::misc::MAX_SHADERS> shaders;
inline std::array<types::Texture, constants::misc::MAX_TEXTURES> textures;

inline bool init = false;

}

