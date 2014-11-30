CXX = g++
CXXFLAGS = -Wall -std=c++0x

LDLIBS = -lglfw3 -lGLEW -lsoil2
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
  LDLIBS += -lGLU -lGL 
  LDLIBS += -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lXinerama -lXcursor -lm
endif
ifeq ($(UNAME_S), Darwin)
  LDLIBS += -framework OpenGL -framework GLUT -framework CoreFoundation -framework Foundation -framework IOKit -framework Quartz -framework AppKit
endif

OBJ_DIR = bin
LIB_DIR = -L ~/Documents/opengl/soil2/lib/macosx -L ~/Documents/opengl/glew/lib -L ~/Documents/opengl/glfw/src
INC_DIR = -I ~/Documents/opengl/soil2/src/SOIL2 -I ~/Documents/opengl/glew/include -I ~/Documents/opengl/glfw/include/GLFW -I ~/Documents/opengl/glm

SOURCE = tutorial01.cc
OBJECTS = ${SOURCE:%.cc=$(OBJ_DIR)/%.o}
EXECUTABLE = tutorial01

all: init $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(LIB_DIR) -o $@ $(OBJECTS) $(LDLIBS)

$(OBJ_DIR)/%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INC_DIR) -c $< -o $@

init:
	@mkdir -p "$(OBJ_DIR)"

clean:
	@rm -rf $(OBJ_DIR) $(EXECUTABLE)

