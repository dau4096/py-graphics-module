PYTHON      := python3
CXX         := g++
CXXFLAGS    := -O3 -Wall -std=c++23 -fPIC
LDFLAGS     := -shared

PYBIND_INC  := $(shell $(PYTHON) -m pybind11 --includes)
PYEXT       := $(shell $(PYTHON)-config --extension-suffix)

GLFW_CFLAGS := $(shell pkg-config --cflags glfw3)
GLFW_LIBS   := $(shell pkg-config --libs glfw3)

OPENGL_LIBS := -lGL

TARGET      := gl$(PYEXT)
SOURCES     := gl.cpp src/graphics.cpp
OBJECTS     := $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ \
	    $(GLFW_LIBS) $(OPENGL_LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(PYBIND_INC) $(GLFW_CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)
