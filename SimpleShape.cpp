// emcc SimpleShape.cpp -o "index.html" -s USE_GLFW=3 -s FULL_ES3=1 -s WASM=1

// ref: https://gist.github.com/ousttrue/0f3a11d5d28e365b129fe08f18f4e141#file-main-cpp

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <GLES3/gl3.h> // For WebGL (OpenGL ES 3.0)
#else
#include <glad/glad.h> // For desktop OpenGL
#endif
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>
#include <functional>

// Vertex Shader
const char* vertexShaderSourceCore = R"(
layout (location = 0) in vec3 aPos;
uniform float rotation;
void main() {
    mat2 rotationMatrix = mat2(cos(rotation), -sin(rotation), sin(rotation), cos(rotation));
    vec2 rotatedPos = rotationMatrix * aPos.xy;
    gl_Position = vec4(rotatedPos, aPos.z, 1.0);
}
)";

// Fragment Shader
const char* fragmentShaderSourceCore = R"(
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 1.0, 0.0, 1.0);  // Yellow color
}
)";

#ifdef __EMSCRIPTEN__
// Add WebGL specific version and precision qualifiers
std::string vertexShaderSource = "#version 300 es\nprecision mediump float;\n" + std::string(vertexShaderSourceCore);
std::string fragmentShaderSource = "#version 300 es\nprecision mediump float;\n" + std::string(fragmentShaderSourceCore);
#else
// Use OpenGL 3.3 core version for desktop
std::string vertexShaderSource = "#version 330 core\n" + std::string(vertexShaderSourceCore);
std::string fragmentShaderSource = "#version 330 core\n" + std::string(fragmentShaderSourceCore);
#endif

const char* vertexShaderSourceCStr = vertexShaderSource.c_str();
const char* fragmentShaderSourceCStr = fragmentShaderSource.c_str();

GLuint shaderProgram;
GLuint VAO;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
//#endif

void render();
void init();

std::function<void()> loop;
void main_loop() { loop(); }

int main() {

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure GLFW for OpenGL 3.3 core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Rotating Triangle", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the context current
    glfwMakeContextCurrent(window);

#ifndef __EMSCRIPTEN__
    // Load OpenGL functions with GLAD
    if (!gladLoadGL()) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
#endif

    // Set the viewport and callback for resizing
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize the shaders, buffers, etc.
    init();

    loop = [&] {
        processInput(window);
        render();
        glfwPollEvents();
        glfwSwapBuffers(window);
    };

#ifdef __EMSCRIPTEN__
    // Emscripten requires a main loop function pointer
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    // Main render loop for desktop OpenGL
    while (!glfwWindowShouldClose(window)) {
        main_loop();
    }
#endif

    glfwTerminate();

    return 0;
}

void checkShaderCompile(GLuint shader) {
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void init() {
    // Build and compile the shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSourceCStr, nullptr);
    glCompileShader(vertexShader);
    checkShaderCompile(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSourceCStr, nullptr);
    glCompileShader(fragmentShader);
    checkShaderCompile(fragmentShader);

    shaderProgram = glCreateProgram();  
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Set up vertex data for a triangle
    float vertices[] = {
         0.0f,   0.575f,  0.0f,
        -0.25f,  0.145f,  0.0f,
         0.25f,  0.145f,  0.0f,

        -0.25f,  0.145f,  0.0f,
        -0.5f,  -0.285f,  0.0f,
         0.0f,  -0.285f,  0.0f,

         0.25f,  0.145f,  0.0f,
         0.0f,  -0.285f,  0.0f,
         0.5f,  -0.285f,  0.0f
    };

    GLuint VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void render() {
    // Clear the screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the shader program
    glUseProgram(shaderProgram);

    // Calculate rotation based on time
    float timeValue = glfwGetTime();
    float rotationAngle = timeValue;  // Rotate based on time

    // Pass the rotation angle to the vertex shader
    int vertexColorLocation = glGetUniformLocation(shaderProgram, "rotation");
    glUniform1f(vertexColorLocation, rotationAngle);

    // Draw the triangle
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 9);
}
