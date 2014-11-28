CXX = g++
CXXFLAGS = -Wall -std=c++0x
LDLIBS = -lglfw3 -lGLEW -lGLU -lGL -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lXinerama -lXcursor -lm

OBJ_DIR = bin
LIB_DIR = -L ~/Documents/glew/lib -L ~/Documents/glfw/src
INC_DIR = -I ~/Documents/glew/include -I ~/Documents/glfw/include/GLFW -I ~/Documents/glm

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

