#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>

// Camera variables
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// Default camera values for reset
glm::vec3 defaultCameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 defaultCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
float defaultYaw = -90.0f;
float defaultPitch = 0.0f;

// Camera rotation variables
float yaw = -90.0f;
float pitch = 0.0f;

// Sphere rotation variables
float sphereRotationX = 0.0f;
float sphereRotationY = 0.0f;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Simple vertex shader
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
)";

// Simple fragment shader
const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 objectColor;
    void main() {
        FragColor = vec4(objectColor, 1.0);
    }
)";

// Simple Sphere class
class Sphere {
private:
    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    float radius;
    int indexCount;

public:
    Sphere(float r, int sectors = 32, int stacks = 32) : radius(r), indexCount(0) {
        createSphereGeometry(sectors, stacks);
        setupBuffers();
        std::cout << "Sphere created with radius: " << r << std::endl;
    }

    ~Sphere() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    void setRadius(float r) { radius = r; }
    float getRadius() const { return radius; }

    void render(unsigned int shaderProgram, float rotX = 0.0f, float rotY = 0.0f) {
        glBindVertexArray(VAO);
        
        // Set model matrix with rotation and scale
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X
        model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y
        model = glm::scale(model, glm::vec3(radius));
        
        // Set uniforms
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 0.5f, 0.2f); // Orange color
        
        // Draw the sphere
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }

private:
    void createSphereGeometry(int sectors, int stacks) {
        float sectorStep = 2 * M_PI / sectors;
        float stackStep = M_PI / stacks;
        
        // Generate vertices
        for (int i = 0; i <= stacks; ++i) {
            float stackAngle = M_PI / 2 - i * stackStep;
            float xy = cosf(stackAngle);
            float z = sinf(stackAngle);
            
            for (int j = 0; j <= sectors; ++j) {
                float sectorAngle = j * sectorStep;
                
                float x = xy * cosf(sectorAngle);
                float y = xy * sinf(sectorAngle);
                
                // Position (unit sphere)
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
            }
        }
        
        // Generate indices
        for (int i = 0; i < stacks; ++i) {
            int k1 = i * (sectors + 1);
            int k2 = k1 + sectors + 1;
            
            for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
                if (i != 0) {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }
                
                if (i != (stacks - 1)) {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }
        
        indexCount = indices.size();
        std::cout << "Generated " << vertices.size() / 3 << " vertices and " << indexCount << " indices" << std::endl;
    }

    void setupBuffers() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        std::cout << "Buffers setup complete" << std::endl;
    }
};

// Helper function to update camera front vector
void updateCameraFront() {
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// Camera control functions
void processInput(GLFWwindow *window) {
    float cameraSpeed = static_cast<float>(2.5 * deltaTime);
    float rotationSpeed = static_cast<float>(50.0 * deltaTime); // Degrees per second
    
    // W/S for zoom in/out (forward/backward)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    
    // A/D for rotation (clockwise/counterclockwise around current view)
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        // Rotate counterclockwise (left)
        yaw -= rotationSpeed;
        updateCameraFront();
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        // Rotate clockwise (right)
        yaw += rotationSpeed;
        updateCameraFront();
    }
    
    // Arrow keys for movement
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;
    
    // R key to reset view
    static bool rPressed = false;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rPressed) {
        // Reset camera position and rotation
        cameraPos = defaultCameraPos;
        cameraFront = defaultCameraFront;
        yaw = defaultYaw;
        pitch = defaultPitch;
        sphereRotationX = 0.0f;
        sphereRotationY = 0.0f;
        std::cout << "View reset to default" << std::endl;
        rPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
        rPressed = false;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    float zoomSpeed = 0.5f;
    cameraPos += cameraFront * static_cast<float>(yoffset) * zoomSpeed;
}

// Function to compile shader
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

// Function to create shader program
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
    GLFWwindow* window = glfwCreateWindow(800, 600, "Simple Sphere", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    // Set up callbacks
    glfwSetScrollCallback(window, scroll_callback);
    
    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    
    // Configure OpenGL
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, 800, 600);
    
    // Create a single sphere
    Sphere sphere(1.0f);
    
    // Compile shaders
    unsigned int vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    
    if (vertexShader == 0 || fragmentShader == 0) {
        std::cout << "Shader compilation failed!" << std::endl;
        return -1;
    }
    
    unsigned int shaderProgram = createShaderProgram(vertexShader, fragmentShader);
    
    if (shaderProgram == 0) {
        std::cout << "Shader program creation failed!" << std::endl;
        return -1;
    }
    
    std::cout << "Shader program created successfully: " << shaderProgram << std::endl;
    
    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Process input
        processInput(window);
        
        // Clear buffers
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Use shader program
        glUseProgram(shaderProgram);
        
        // Set view and projection matrices
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        
        // Render the sphere with rotation
        sphere.render(shaderProgram, sphereRotationX, sphereRotationY);
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Clean up
    glDeleteProgram(shaderProgram);
    
    glfwTerminate();
    return 0;
}
