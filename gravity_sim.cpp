#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "Sphere.h"

// Window dimensions
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Physics configuration
PhysicsConfig physicsConfig;

// Much smaller mass for manageable gravitational effects
const float MOON_MASS = 1e15f;  // 1 million kg (very small mass)

// Gravity simulation variables - Two spheres with orbital velocities
std::vector<SpherePhysics> spheres = {
    SpherePhysics(-1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.4f, MOON_MASS, glm::vec3(0.8f, 0.3f, 0.3f)),  // Red sphere with Z velocity
    SpherePhysics( 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.4f, MOON_MASS, glm::vec3(0.3f, 0.3f, 0.8f))   // Blue sphere with opposite Z velocity
};

// Camera variables
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 8.0f);  // Move camera back and up for better view
glm::vec3 cameraFront = glm::vec3(0.0f, -0.2f, -1.0f);  // Look slightly down
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);



// Shader sources
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec3 FragPos;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main() {
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}
)";

// Global variables
unsigned int shaderProgram;
Sphere* sphere;  // We'll use one sphere geometry for all instances

// Function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
unsigned int compileShader(const char* source, GLenum type);
unsigned int createShaderProgram(unsigned int vertexShader, unsigned int fragmentShader);



int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Gravity Simulation", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    
    // Create sphere geometry (we'll scale it for different sizes)
    sphere = new Sphere(1.0f);  // Unit sphere, we'll scale it per instance
    
    // Compile and create shader program
    unsigned int vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    shaderProgram = createShaderProgram(vertexShader, fragmentShader);
    
    if (shaderProgram == 0) {
        std::cout << "Failed to create shader program" << std::endl;
        return -1;
    }
    
    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Input
        processInput(window);
        
        // Update physics
        Sphere::updatePhysics(spheres, physicsConfig);
        
        // Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Activate shader
        glUseProgram(shaderProgram);
        
        // Set up view and projection matrices
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        
        // Pass matrices to shader
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        
        // Render all spheres using the Sphere class method
        Sphere::renderAllSpheres(spheres, sphere, shaderProgram, 
                                glm::vec3(2.0f, 2.0f, 2.0f),  // lightPos
                                cameraPos,                     // viewPos
                                glm::vec3(1.0f, 1.0f, 1.0f)); // lightColor
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Clean up
    delete sphere;
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // Toggle bounce on/off with B key
    static bool bPressed = false;
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !bPressed) {
        physicsConfig.enableBounce = !physicsConfig.enableBounce;
        std::cout << "Bounce " << (physicsConfig.enableBounce ? "ENABLED" : "DISABLED") << std::endl;
        bPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
        bPressed = false;
    }
    
    // Toggle gravitational attraction with G key
    static bool gPressed = false;
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !gPressed) {
        physicsConfig.enableGravity = !physicsConfig.enableGravity;
        std::cout << "Gravitational attraction " << (physicsConfig.enableGravity ? "ENABLED" : "DISABLED") << std::endl;
        gPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE) {
        gPressed = false;
    }
    
    // Adjust orbital velocity with arrow keys
    static bool upPressed = false;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !upPressed) {
        // Increase orbital velocity for both spheres
        for (auto& sphere : spheres) {
            sphere.velocityZ *= 1.2f;  // Increase by 20%
        }
        std::cout << "Increased orbital velocity" << std::endl;
        upPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) {
        upPressed = false;
    }
    
    static bool downPressed = false;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !downPressed) {
        // Decrease orbital velocity for both spheres
        for (auto& sphere : spheres) {
            sphere.velocityZ *= 0.8f;  // Decrease by 20%
        }
        std::cout << "Decreased orbital velocity" << std::endl;
        downPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) {
        downPressed = false;
    }
    
    // Reset orbital velocities with R key
    static bool rPressed = false;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rPressed) {
        // Reset to initial orbital velocities
        spheres[0].velocityZ = 2.0f;   // Red sphere
        spheres[1].velocityZ = -2.0f;  // Blue sphere
        std::cout << "Reset orbital velocities" << std::endl;
        rPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
        rPressed = false;
    }
    
    // Adjust bounce damping with + and - keys
    static bool plusPressed = false;
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS && !plusPressed) {  // + key
        physicsConfig.bounceDamping = std::min(1.0f, physicsConfig.bounceDamping + 0.1f);
        std::cout << "Bounce damping: " << physicsConfig.bounceDamping << std::endl;
        plusPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_RELEASE) {
        plusPressed = false;
    }
    
    static bool minusPressed = false;
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS && !minusPressed) {  // - key
        physicsConfig.bounceDamping = std::max(0.0f, physicsConfig.bounceDamping - 0.1f);
        std::cout << "Bounce damping: " << physicsConfig.bounceDamping << std::endl;
        minusPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_RELEASE) {
        minusPressed = false;
    }
}

unsigned int compileShader(const char* source, GLenum type) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        return 0;
    }
    
    return shader;
}

unsigned int createShaderProgram(unsigned int vertexShader, unsigned int fragmentShader) {
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return 0;
    }
    
    return program;
}
