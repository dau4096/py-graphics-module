#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "includes.h"
#include "global.h"
#include "utils.h"


namespace graphics {

	void init(std::string name, std::pair<int, int> resolution, std::pair<int, int> version);
	void terminate();

}


#endif

