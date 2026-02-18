#pragma once
#include "includes.h"
#include "constants.h"
#include "utils.h"



namespace types {


//Version of OpenGL.
struct GLVersion {
	unsigned int major = 0u;
	unsigned int minor = 0u;
	bool embedded = false;

	//Minimum supported versions [3.30 Core+ / 3.1 ES+].
	static constexpr glm::uvec2 MIN_CORE = {3u, 3u};
	static constexpr glm::uvec2 MIN_ES   = {3u, 1u};

	GLVersion(glm::uvec2 ver, bool es)
		: major(ver.x), minor(ver.y), embedded(es) {}

	constexpr bool operator>=(const GLVersion& other) const {
		return (
			(major > other.major) ||
			((major == other.major) && (minor >= other.minor))
		);
	}

	//Is this version new enough?
	bool valid() const {
		const glm::uvec2 minVer = (embedded) ? MIN_ES : MIN_CORE;
		GLVersion minVersion = GLVersion(minVer, embedded);
		return *this >= minVersion;
	}

	//Tell GLFW to use this version.
	void use() const {
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
		glfwWindowHint(GLFW_CLIENT_API, ((embedded) ? GLFW_OPENGL_ES_API : GLFW_OPENGL_API));
	}
};



//Different types of uniform accepted.
using UniformStorage = std::variant<
    float,
    int,
    glm::vec2, glm::ivec2, //2D vectors
    glm::vec3, glm::ivec3, //3D vectors
    glm::vec4, glm::ivec4, //4D vectors
    glm::mat3, glm::mat4   //Matrices
>;

//Inside 1 struct type.
struct UniformValue {
    UniformType type = UV_INVAL;
    UniformStorage data;

    UniformValue() : type(UV_INVAL), data(0.0f) {}
    UniformValue(float v)      : type(UV_FLOAT), data(v) {}
    UniformValue(int v)        : type(UV_INTEG), data(v) {}
    UniformValue(bool v)       : type(UV_INTEG), data(v ? 1 : 0) {}
    UniformValue(glm::vec2 v)  : type(UV_FVEC2), data(v) {}
    UniformValue(glm::ivec2 v) : type(UV_IVEC2), data(v) {}
    UniformValue(glm::vec3 v)  : type(UV_FVEC3), data(v) {}
    UniformValue(glm::ivec3 v) : type(UV_IVEC3), data(v) {}
    UniformValue(glm::vec4 v)  : type(UV_FVEC4), data(v) {}
    UniformValue(glm::ivec4 v) : type(UV_IVEC4), data(v) {}
    UniformValue(glm::mat3 v)  : type(UV_MAT33), data(v) {}
    UniformValue(glm::mat4 v)  : type(UV_MAT44), data(v) {}
};



//Contains data related to calling a shader.
//ST_COMPUTE     → localSize
//ST_WORLDSPACE  → VAO & index count
//ST_SCREENSPACE → N/A
struct ShaderCall {
	glm::uvec3 localSize = glm::uvec3(0u, 0u, 0u);
	GLuint VAO = 0u;
	unsigned int numberOfIndices = 0u;
	bool hasVAO = false;

	ShaderCall() : localSize(0u, 0u, 0u), VAO(0u), numberOfIndices(0u), hasVAO(false) {}
	ShaderCall(glm::uvec3& ls) : localSize(ls), VAO(0u), numberOfIndices(0u), hasVAO(false) {}
	ShaderCall(GLuint vao, unsigned int nv)	: localSize(0u, 0u, 0u), VAO(vao), numberOfIndices(nv), hasVAO(true) {}
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
			utils::cout(std::format("\nShader [{}] compile error:", name));
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




class Texture {
private:
	bool _valid = false;

public:
	std::string name = ""; //Used for binding in shaders.
	bool sampler2D = false;
	std::string filePath = "";
	glm::ivec2 resolution = {0, 0};
	int channels = 0;
	GLuint GLindex = 0;
	std::pair<GLint, GLint> minMagFilters;
	std::pair<GLint, GLint> wrap;
	GLint format = 0;


	Texture() = default;
	Texture(const std::string n, glm::ivec2 res)
		: name(n), sampler2D(false), resolution(res), channels(4), format(GL_RGBA32F) {}
	Texture(const std::string n, const std::string& path, glm::ivec2 res, int ch, GLint fmt)
		: name(n), sampler2D(true), filePath(path), resolution(res), channels(ch), format(fmt) {}

	void setValid(bool validity) {_valid = validity;}
	bool isValid() const {return _valid;}

	void label() {
		if (GLEW_KHR_debug || GLEW_VERSION_4_3) {glObjectLabel(GL_TEXTURE, GLindex, -1, name.c_str()); /* Label it for debugging. */}
	}

	//Deletion
	void destroy() {
		_valid = false;
		sampler2D = false;
		resolution = {0, 0};
		int channels = 0;
		filePath = ""; name = "";
		minMagFilters = {}; wrap = {};
		format = 0;

		//Free texture from OpenGL.
		glDeleteTextures(1, &GLindex);
		GLindex = 0;
	}
	~Texture() {destroy();}
};


//Contains the values required to bind a texture in a shader.
struct BoundTexture {
	GLuint GLindex = 0u;
	std::string name = "";
	bool isValid = false;
	bool isSampler2D = false;
	GLint format = 0;

	BoundTexture() : isValid(false) {}
	BoundTexture(Texture& texture)
		: GLindex(texture.GLindex), name(texture.name), isValid(true),
		  isSampler2D(texture.sampler2D), format(texture.format) {}
};



//Full shader program.
class ShaderProgram {
private:
	GLuint _program = 0u; //OpenGL index
	bool _linked = false; //Ready to be used or not
	std::unordered_map<std::string, UniformValue> _uniforms; //Uniforms and their names to bind
	std::unordered_map<GLuint, BoundTexture> _textures; //GL Indices & data of textures to bind at runtime.
	ShaderCall _call; //Contains data to be used when doing shader.run();

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
	void destroy() {
		if (_program) {glDeleteProgram(_program);}

		_program = 0u;
		_linked = false;
		_uniforms = {};
		_textures = {};
		_call = ShaderCall();
		type = ST_NONE;
	}
	~ShaderProgram() {destroy();}


	bool createProgram(std::vector<ShaderObject>& shaders, ShaderType type) {
		_program = glCreateProgram();
		for (ShaderObject& sh : shaders) {
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
			utils::cout(std::format("Found ST_COMPUTE local size of: [{}, {}, {}]", workGroupSize[0], workGroupSize[1], workGroupSize[2]));

			_call.localSize = glm::uvec3(
				static_cast<unsigned int>(workGroupSize[0]),
				static_cast<unsigned int>(workGroupSize[1]),
				static_cast<unsigned int>(workGroupSize[2])
			);
		}


		for (ShaderObject& sh : shaders) {
			//Delete shader, it has been used.
			sh.destroy();
		}

		_linked = true;
		this->type = type;
		return true;
	}


	void setVAO(VAOFormat format, std::vector<float>& vertices, std::vector<GLuint>& indices) {
		//Create VAO with given format and values.
		std::unordered_map<VAOFormat, std::string> formatNameMap = std::unordered_map<VAOFormat, std::string>{
			{VAO_EMPTY, "VAO_EMPTY"}, 						{VAO_POS_ONLY, "VAO_POS_ONLY"},				 {VAO_POS_NORMAL, "VAO_POS_NORMAL"},
			{VAO_POS_UV2D, "VAO_POS_UV2D"}, 				{VAO_POS_UV3D, "VAO_POS_UV3D"},
			{VAO_POS_UV2D_NORMAL, "VAO_POS_UV2D_NORMAL"}, 	{VAO_POS_UV3D_NORMAL, "VAO_POS_UV3D_NORMAL"}
		};
		utils::cout(std::format(
			"Creating VAO with format [{}]", formatNameMap[format]
		));
		if (format == VAO_EMPTY) {
			_call.numberOfIndices = 0u;
			_call.VAO = constants::display::emptyVAO;
			return;
		}
		size_t vertexSizeSingular = constants::display::vertexFormatSizeMap.at(format);
		_call.numberOfIndices = indices.size();

	
		glGenVertexArrays(1, &(_call.VAO));
		glBindVertexArray(_call.VAO);

		GLuint VBO, EBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW); //Reserve space

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _call.numberOfIndices * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW); //Reserve space


		size_t offset = 0u;
		unsigned int attribID = 0;
		//Loop through and set attributes based on the format. For instance, VAO_POS_UV2D_NORMAL = {3, 2, 3}.
		for (const Attribute& attr : (constants::display::layouts.at(format))) {
			utils::cout(V_DEBUG, std::format("Adding attribute with size: [{} VALUES, {} BYTES]", attr.size, attr.size*sizeof(float)));
			glVertexAttribPointer(attribID, attr.size, GL_FLOAT, GL_FALSE, vertexSizeSingular * sizeof(float), (void*)(offset * sizeof(float)));
			glEnableVertexAttribArray(attribID++);
			offset += attr.size;
		}

		glBindVertexArray(0);
		_call.hasVAO = true;
	}


	inline void use() {if (_linked) {glUseProgram(_program);} else {utils::cerr("Must create shader first, before using it.");}}


	template<typename T>
	void setUniform(const std::string& name, const T& val) {
		utils::cout(std::format("Setting uniform value with Name=\"{}\"", name));
		_uniforms[name] = UniformValue(val);
	}


	void applyUniforms() {
		for (const auto& [name, u] : _uniforms) {
			GLint loc = glGetUniformLocation(_program, name.c_str());
			if (loc == -1) {continue; /* Invalid location for this shader. */}
			utils::cout(V_DEBUG, std::format("Applying uniform with Name=\"{}\"", name));

			switch (u.type) {
				case UniformType::UV_FLOAT: {float      data = std::get<float>(u.data);      glUniform1f(loc, data);  break;}
				case UniformType::UV_INTEG: {int        data = std::get<int>(u.data);        glUniform1i(loc, data);  break;}
				case UniformType::UV_FVEC2: {glm::vec2  data = std::get<glm::vec2>(u.data);  glUniform2f(loc, data.x, data.y); break;}
				case UniformType::UV_IVEC2: {glm::ivec2 data = std::get<glm::ivec2>(u.data); glUniform2i(loc, data.x, data.y); break;}
				case UniformType::UV_FVEC3: {glm::vec3  data = std::get<glm::vec3>(u.data);  glUniform3f(loc, data.x, data.y, data.z); break;}
				case UniformType::UV_IVEC3: {glm::ivec3 data = std::get<glm::ivec3>(u.data); glUniform3i(loc, data.x, data.y, data.z); break;}
				case UniformType::UV_FVEC4: {glm::vec4  data = std::get<glm::vec4>(u.data);  glUniform4f(loc, data.x, data.y, data.z, data.w); break;}
				case UniformType::UV_IVEC4: {glm::ivec4 data = std::get<glm::ivec4>(u.data); glUniform4i(loc, data.x, data.y, data.z, data.w); break;}
				case UniformType::UV_MAT33: {glm::mat3  data = std::get<glm::mat3>(u.data);  glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(data)); break;}
				case UniformType::UV_MAT44: {glm::mat4  data = std::get<glm::mat4>(u.data);  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(data)); break;}
				default: {break;}
			}
		}
	}


	bool run(glm::uvec3 dispatchSize, unsigned int shaderID) {
		switch (this->type) {
			case ST_COMPUTE: {
				//Dispatch compute
				utils::cout(std::format(
					"Running Compute shader ID [{}] with dispatch size [{}, {}, {}]",
					shaderID, dispatchSize.x,
					dispatchSize.y,	dispatchSize.z
				));
				glm::uvec3 dispatchGroups = (dispatchSize + _call.localSize - 1u) / _call.localSize;
				glDispatchCompute(dispatchGroups.x, dispatchGroups.y, dispatchGroups.z);
				break;
			}

			case ST_WORLDSPACE: {
				//Run worldspace (3D)
				utils::cout(std::format(
					"Running Worldspace [3D] shader ID [{}]",	shaderID
				));
				if (!_call.hasVAO) {
					//No VAO added.
					utils::cerr(std::format("No vertices were bound to the shader. Use \"gl.add_vao(shaderID, format, values)\" where shaderID=[{}]", shaderID));
					return false;
				}
				glBindVertexArray(_call.VAO);
				glDrawElements(GL_TRIANGLES, _call.numberOfIndices, GL_UNSIGNED_INT, nullptr);
				glBindVertexArray(0);
				break;
			}

			case ST_SCREENSPACE: {
				//Run screenspace (2D)
				utils::cout(std::format(
					"Running Screenspace [2D] shader ID [{}]",	shaderID
				));
				glBindVertexArray(constants::display::emptyVAO); //No VAO needed, uses vertices from the vertex-shader.
				glDrawArrays(GL_TRIANGLE_STRIP, 0u, 4u);
				glBindVertexArray(0);
				break;
			}
			default: {return false; /* Unknown type */}
		}
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		return true;
	}


	bool bindTexture(GLuint binding, Texture& texture) {
		if (!texture.isValid()) {
			utils::cerr("Tried to bind invalid texture");
			return false;
		}

		_textures[binding] = BoundTexture(texture); //TBA - implement BoundTexture struct. [NEEDS: isSampelr2D[bool], GLindex[GLuint], format[GLint]]

		return true;
	}


	void applyTextures() {
		for (const auto& [binding, bTex] : _textures) {
			if (bTex.isSampler2D) {
				utils::cout(V_DEBUG, std::format("Applying texture with Name=\"{}\"", bTex.name));
				glBindTextureUnit(binding, bTex.GLindex);
			} else {
				utils::cout(V_DEBUG, std::format("Applying image with Name=\"{}\"", bTex.name));
				glBindImageTexture(
					binding, bTex.GLindex,
					0, GL_FALSE, 0,
					GL_READ_WRITE, bTex.format
				);
			}
		}
	}
};



class Camera {
private:
	bool _valid = false;

public:
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 angle = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 forward = glm::vec3(0.0f, 1.0f, 0.0f); 	//Forward vector.
	glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f); 		//Right vector.
	glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f); 		//Up vector.

	float FOV = 0.0f; //Radians
	float nearZ = -1.0f; //Near plane
	float farZ = 1.0f; //Far plane

	void assign(glm::vec3 p, glm::vec3 a, glm::vec3 u, float fov, float nz, float fz) {
		position = p; angle = a;
		up = u;	FOV = fov;
		nearZ = nz;	farZ = fz;
		_valid = true;
	}

	bool isValid() {return _valid;}

	//Deletion
	void destroy() {
		//Set invalid, and reset to defaults.
		_valid = false;
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		angle = glm::vec3(0.0f, 0.0f, 0.0f);
		up = glm::vec3(0.0f, 0.0f, 1.0f);
		FOV = 0.0f;
		nearZ = -1.0f;
		farZ = 1.0f;
	}
	~Camera() {destroy();}
};

}



namespace shared {

//Shared GL resources
inline GLFWwindow* window = nullptr;

//Current numbers, for assigning new entries.
inline size_t numberOfShaders;
inline size_t numberOfTextures;
inline size_t numberOfCameras;
//Respective datasets;
inline std::array<types::ShaderProgram, constants::misc::MAX_SHADERS> shaders; //All shaders the user has loaded
inline std::array<types::Texture, constants::misc::MAX_TEXTURES> textures;     //All textures the user may bind / write to
inline std::array<types::Camera, constants::misc::MAX_CAMERAS> cameras;        //All cameras the user controls

inline bool init = false;
inline glm::ivec2 windowResolution;

}

