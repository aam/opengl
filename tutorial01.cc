// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
using namespace glm;

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
  const GLubyte* renderer = glGetString (GL_RENDERER); // get renderer string
  const GLubyte* version = glGetString (GL_VERSION); // version as a string
  printf ("Renderer: %s\n", renderer);
  printf ("OpenGL version supported %s\n", version);

  // tell GL to only draw onto a pixel if the shape is closer to the viewer
  glEnable (GL_DEPTH_TEST); // enable depth-testing
  glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  // Dark blue background
  glClearColor(.0f, .0f, 0.4f, 0.0f);

  GLuint VertexArrayID = 0;
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  // Create and compile our GLSL program from the shaders
  GLuint programID = LoadShaders("SimpleVertexShader.vertexshader",
                                 "SimpleFragmentShader.fragmentshader");

  GLuint MatrixID = glGetUniformLocation(programID, "MVP");

  // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
  glm::mat4 Projection = glm::perspective(60.0f, 4.0f / 3.0f, 0.1f, 100.0f);
  // Camera matrix
  glm::mat4 View       = glm::lookAt(
      glm::vec3(4,3,-3), // Camera is at (4,3,3), in World Space
      glm::vec3(0,0,0), // and looks at the origin
      glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
  );
  // Model matrix : an identity matrix (model will be at the origin)
  glm::mat4 Model      = glm::mat4(1.0f);  // Changes for each model !
  // Our ModelViewProjection : multiplication of our 3 matrices
  glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around

  static const GLfloat g_vertex_buffer_data[] = {
      -1.0f,-1.0f,-1.0f, // triangle 1 : begin
      -1.0f,-1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f, // triangle 1 : end
      1.0f, 1.0f,-1.0f, // triangle 2 : begin
      -1.0f,-1.0f,-1.0f,
      -1.0f, 1.0f,-1.0f, // triangle 2 : end
      1.0f,-1.0f, 1.0f,
      -1.0f,-1.0f,-1.0f,
      1.0f,-1.0f,-1.0f,
      1.0f, 1.0f,-1.0f,
      1.0f,-1.0f,-1.0f,
      -1.0f,-1.0f,-1.0f,
      -1.0f,-1.0f,-1.0f,
      -1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f,-1.0f,
      1.0f,-1.0f, 1.0f,
      -1.0f,-1.0f, 1.0f,
      -1.0f,-1.0f,-1.0f,
      -1.0f, 1.0f, 1.0f,
      -1.0f,-1.0f, 1.0f,
      1.0f,-1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f,-1.0f,-1.0f,
      1.0f, 1.0f,-1.0f,
      1.0f,-1.0f,-1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f,-1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f,-1.0f,
      -1.0f, 1.0f,-1.0f,
      1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f,-1.0f,
      -1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f,
      1.0f,-1.0f, 1.0f,
  };

  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
               g_vertex_buffer_data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator(seed);
  std::uniform_real_distribution<float> distribution(0, 1);
  static GLfloat g_color_buffer_data[12*3*3];

  GLuint colorbuffer;
  glGenBuffers(1, &colorbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
  for (int v = 0; v < 12*3 ; v++){
    g_color_buffer_data[3*v+0] = distribution(generator);
    g_color_buffer_data[3*v+1] = distribution(generator);
    g_color_buffer_data[3*v+2] = distribution(generator);
  }
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data),
               g_color_buffer_data, GL_STATIC_DRAW);
//  glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
  glm::mat4 ProjectionTri = glm::perspective(60.0f, 4.0f / 3.0f, 0.1f, 100.0f);
  // Camera matrix
  glm::mat4 ViewTri       = glm::lookAt(
      glm::vec3(0,3,-3), // Camera is at (4,3,3), in World Space
      glm::vec3(0,0,0), // and looks at the origin
      glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
  );
  // Model matrix : an identity matrix (model will be at the origin)
  glm::mat4 ModelTri      = glm::mat4(2.0f);  // Changes for each model !

  glm::mat4 MVP_tri    = ProjectionTri * ViewTri * ModelTri; // Remember, matrix multiplication is the other way around

  static const GLfloat g_vertex_buffer_data_tri[] = {
      -0.5f, -0.5f, -2.0f,
      0.5f, -0.5f, -2.0f,
      0.0f, 0.5f, -2.0f,
  };

  GLuint vertexbuffer_tri;
  glGenBuffers(1, &vertexbuffer_tri);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_tri);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data_tri),
               g_vertex_buffer_data_tri, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  static GLfloat g_color_buffer_data_tri[1*3*3];

  GLuint colorbuffer_tri;
  glGenBuffers(1, &colorbuffer_tri);
  glBindBuffer(GL_ARRAY_BUFFER, colorbuffer_tri);
  for (int v = 0; v < 1*3 ; v++){
    g_color_buffer_data_tri[3*v+0] = distribution(generator);
    g_color_buffer_data_tri[3*v+1] = distribution(generator);
    g_color_buffer_data_tri[3*v+2] = distribution(generator);
  }
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data_tri),
               g_color_buffer_data_tri, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  do {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(programID);

    for (int v = 0; v < 12*3 ; v++){
      g_color_buffer_data[3*v+0] = distribution(generator);
      g_color_buffer_data[3*v+1] = distribution(generator);
      g_color_buffer_data[3*v+2] = distribution(generator);
    }
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data),
                 g_color_buffer_data, GL_STATIC_DRAW);

    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(VertexArrayID);

    glDrawArrays(GL_TRIANGLES, 0, 12*3);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    for (int v = 0; v < 1*3 ; v++){
      g_color_buffer_data_tri[3*v+0] = distribution(generator);
      g_color_buffer_data_tri[3*v+1] = distribution(generator);
      g_color_buffer_data_tri[3*v+2] = distribution(generator);
    }
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data_tri),
                 g_color_buffer_data_tri, GL_STATIC_DRAW);


    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP_tri[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_tri);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer_tri);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(VertexArrayID);

    glDrawArrays(GL_TRIANGLES, 0, 1*3);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);


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

