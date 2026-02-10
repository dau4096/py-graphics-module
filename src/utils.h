#ifndef UTILS_H
#define UTILS_H

#include "includes.h"
#include "constants.h"


namespace utils {


template<typename... Args>
static inline void cout_inline(Args&&... args) {
	if (!shared::verbose) {return; /* Don't output to console. */}
	((std::cout << args << " "), ...); //Requires C++17 or more.
	std::cout << std::flush;
}
template<typename... Args>
static inline void cout(Args&&... args) {
	if (!shared::verbose) {return; /* Don't output to console. */}
	((std::cout << args << " "), ...); //Requires C++17 or more.
	std::cout << std::endl;
}

template<typename... Args>
static inline void cerr(Args&&... args) {
	//Concatenate
	std::ostringstream oss;
	((oss << args << " "), ...);

	//Raise exception.
	throw std::runtime_error(oss.str());
}




static inline void pause() {
	std::string pause;
	std::cin >> pause;
}

static inline void GLErrorcheck(std::string location="", bool shouldPause=false) {
	GLenum GLError;
	GLError = glGetError();
	if (GLError != GL_NO_ERROR) {
		utils::cerr(location, " | OpenGL error; ", GLError, "\n");
		if (shouldPause) {pause();}
	}
}



static std::string readFile(const std::string& filePath) {
	std::ifstream fileStream(filePath, std::ios::binary);
	if (!fileStream.is_open()) {
		cerr("Error: Could not open file: ", filePath, "\n");
	}

	std::ostringstream buffer;
	buffer << fileStream.rdbuf();
	return buffer.str();
}

static std::string getFilename(const std::string& path) {
	size_t pos = path.find_last_of("/\\"); //Windows and linux style (/ and \)
	if (pos == std::string::npos) {return path; /* None found */}
	return path.substr(pos + 1);
}



static inline std::string getTimestamp() {
	time_t now = time(nullptr);
	struct tm* timeinfo = localtime(&now);

	std::ostringstream oss;
	oss << std::put_time(timeinfo, "%Y%m%d%H%M%S");

	return oss.str();
}


}

#endif
