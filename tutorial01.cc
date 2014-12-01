// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <sys/timeb.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <SOIL2.h>

#include <Halide.h>

Halide::Image<int32_t> halideGenerate() {
  Halide::Func gradient;
  Halide::Var x, y;
  Halide::Expr e = select(abs((y * y) - (x * 1024)) < 10 * 1024, 0xff, 0x00);
  Halide::Expr eup = select(
      abs(((1024 - y) * (1024 - y)) - (x * 1024)) < 10 * 1024, 0xff, 0x00);
  gradient(x, y) = select(e > 0 || eup > 0, e + eup, 0);
  Halide::Image<int32_t> output = gradient.realize(1024, 1024);
  return output;
}

void error_callback(int error, const char* description) {
  fputs(description, stderr);
}

GLuint LoadShaders(const char* vertex_file_path,
                   const char* fragment_file_path) {
  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Read the Vertex Shader code from the file
  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
  if (VertexShaderStream.is_open()) {
    std::string Line = "";
    while (getline(VertexShaderStream, Line)) VertexShaderCode += "\n" + Line;
    VertexShaderStream.close();
  }

  // Read the Fragment Shader code from the file
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
  if (FragmentShaderStream.is_open()) {
    std::string Line = "";
    while (getline(FragmentShaderStream, Line))
      FragmentShaderCode += "\n" + Line;
    FragmentShaderStream.close();
  }

  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  printf("Compiling shader : %s\n", vertex_file_path);
  char const* VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> VertexShaderErrorMessage(InfoLogLength);
  glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL,
                     &VertexShaderErrorMessage[0]);
  fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

  // Compile Fragment Shader
  printf("Compiling shader : %s\n", fragment_file_path);
  char const* FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
  glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL,
                     &FragmentShaderErrorMessage[0]);
  fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

  // Link the program
  fprintf(stdout, "Linking program\n");
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> ProgramErrorMessage(max(InfoLogLength, int(1)));
  glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
  fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return ProgramID;
}

glm::vec3 ObjectCenter = glm::vec3(0, 0, 0);
glm::vec3 position = glm::vec3(0, 0, 1);
float FoV = 90.0f;

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

int getMilliCount() {
  timeb tb;
  ftime(&tb);
  int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
  return nCount;
}

int getMilliSpan(int nTimeStart) {
  int nSpan = getMilliCount() - nTimeStart;
  if (nSpan < 0) nSpan += 0x100000 * 1000;
  return nSpan;
}

double gScrollY = 0.0;
void OnScroll(GLFWwindow* window, double deltaX, double deltaY) {
  gScrollY += deltaY;
}

void computeMatricesFromInputs() {
  static const long double startTime = getMilliCount();
  const long double span = getMilliSpan(startTime) / 10;

  static long double pos = 0;
  pos = span;

  position =
      ObjectCenter + glm::vec3(cos(pos / 1000) * cos(pos / 1000) / 2, 0, 1);

  const float zoomSensitivity = -10.0f;
  float fieldOfView = FoV + zoomSensitivity * (float)gScrollY;
  if (fieldOfView < 5.0f) fieldOfView = 5.0f;
  if (fieldOfView > 130.0f) fieldOfView = 130.0f;
  FoV = fieldOfView;
  gScrollY = 0;

  // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit
  // <-> 100 units
  ProjectionMatrix = glm::perspective(FoV, 3.0f / 3.0f, 0.1f, 100.0f);
  // Camera matrix
  ViewMatrix = glm::lookAt(
      position, glm::vec3(cos(pos / 100) * cos(pos / 100) * 0.01, 0, 0),
      glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
      );
}

int main(void) {
  glfwSetErrorCallback(error_callback);

  // Initialise GLFW
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return -1;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef __APPLE__
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

  // Open a window and create its OpenGL context
  window = glfwCreateWindow(640, 480, "Tutorial 01. Or is it?", NULL, NULL);
  if (window == NULL) {
    fprintf(stderr,
            "Failed to open GLFW window. If you have an Intel GPU, they are "
            "not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  glewExperimental = GL_TRUE;
  // Initialize GLEW
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    return -1;
  }

  // get version info
  const GLubyte* renderer = glGetString(GL_RENDERER);  // get renderer string
  const GLubyte* version = glGetString(GL_VERSION);    // version as a string
  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported %s\n", version);

  // tell GL to only draw onto a pixel if the shape is closer to the viewer
  glEnable(GL_DEPTH_TEST);  // enable depth-testing
  glDepthFunc(GL_LESS);  // depth-testing interprets a smaller value as "closer"

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  // Dark blue background
  glClearColor(.0f, .0f, 0.4f, 0.0f);

  GLuint VertexArrayID = 0;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  // Create and compile our GLSL program from the shaders
  GLuint programID = LoadShaders("SimpleVertexShader.vertexshader",
                                 "SimpleFragmentShader.fragmentshader");

  GLuint MatrixID = glGetUniformLocation(programID, "MVP");

  auto output = halideGenerate();

  int width = output.width();
  int height = output.height();
  int imageSize = width * height * 3;
  unsigned char* data = new unsigned char[imageSize];
  std::cout << "width=" << width << " height=" << height << std::endl;

  for (int j = 0; j < output.height(); j++) {
    for (int i = 0; i < output.width(); i++) {
      auto pos = j * output.height() + i;
      data[pos * 3] = output(i, j);
      data[pos * 3 + 1] = 0;
      data[pos * 3 + 2] = 0;
    }
  }

  // Create one OpenGL texture
  GLuint textureID;
  glGenTextures(1, &textureID);
  // "Bind" the newly created texture : all future texture functions will modify
  // this texture
  glBindTexture(GL_TEXTURE_2D, textureID);
  // Give the image to OpenGL
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR,
               GL_UNSIGNED_BYTE, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // When MINifying the image, use a LINEAR blend of two mipmaps, each filtered
  // LINEARLY too
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  // Generate mipmaps, by the way.
  glGenerateMipmap(GL_TEXTURE_2D);

  GLuint Texture = textureID;

  if (Texture == 0)
    std::cerr << "SOIL loading error: '" << SOIL_last_result() << "' ("
              << "img_test.dds"
              << ")" << std::endl;

  // Get a handle for our "myTextureSampler" uniform
  GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

  static const GLfloat g_vertex_buffer_data[] = {
      -1.0f, 1.0f,  0.0f,  // vertex
      1.0f,  1.0f,  0.0f,  // vertex
      1.0f,  -1.0f, 0.0f,  // vertex
      1.0f,  -1.0f, 0.0f,  // vertex
      -1.0f, -1.0f, 0.0f,  // vertex
      -1.0f, 1.0f,  0.0f,  // vertex
  };

  // Two UV coordinatesfor each vertex. They were created withe Blender.
  static const GLfloat g_uv_buffer_data[] = {
      0.0f, 0.0f,  // vertex
      1.0f, 0.0f,  // vertex
      1.0f, 1.0f,  // vertex
      1.0f, 1.0f,  // vertex
      0.0f, 1.0f,  // vertex
      0.0f, 0.0f,  // vertex
  };

  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
               g_vertex_buffer_data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  GLuint uvbuffer;
  glGenBuffers(1, &uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data,
               GL_STATIC_DRAW);

  glfwSetScrollCallback(window, OnScroll);

  do {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(programID);

    // Compute the MVP matrix from keyboard and mouse input
    computeMatricesFromInputs();
    glm::mat4 ModelMatrix = glm::mat4(1.0);
    glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(TextureID, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(1,  // attribute. No particular reason for 1, but must
                              // match the layout in the shader.
                          2,  // size : U+V => 2
                          GL_FLOAT,  // type
                          GL_FALSE,  // normalized?
                          0,         // stride
                          (void*)0   // array buffer offset
                          );
    glBindVertexArray(VertexArrayID);

    glDrawArrays(GL_TRIANGLES, 0, 2 * 3);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();

  }  // Check if the ESC key was pressed or the window was closed
  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         glfwWindowShouldClose(window) == 0);

  // Cleanup VBO and shader
  glDeleteBuffers(1, &vertexbuffer);
  glDeleteProgram(programID);
  glDeleteVertexArrays(1, &VertexArrayID);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  return 0;
}

