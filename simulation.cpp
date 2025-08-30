#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>

// Vertex Shader source code
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    
    out vec3 FragPos;
    out vec3 Normal;
    
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    void main()
    {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        gl_Position = projection * view * vec4(FragPos, 1.0);
    }
)";

// Fragment Shader source code
const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    
    in vec3 FragPos;
    in vec3 Normal;
    
    uniform vec3 lightPos;
    uniform vec3 viewPos;
    uniform vec3 lightColor;
    uniform vec3 objectColor;
    
    void main()
    {
        // ambient
        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * lightColor;
        
        // diffuse
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        
        // specular
        float specularStrength = 0.5;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * lightColor;
        
        vec3 result = (ambient + diffuse + specular) * objectColor;
        FragColor = vec4(result, 1.0);
    }
)";

// Sphere class for easy creation of spheres with different properties
class Sphere {
private:
    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    glm::vec3 position;
    glm::vec3 color;
    float radius;
    int indexCount;

public:
    Sphere(float r, const glm::vec3& pos, const glm::vec3& col, int sectors = 32, int stacks = 32) 
        : radius(r), position(pos), color(col), indexCount(0) {
        std::cout << "Creating sphere with radius: " << r << ", position: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
        createSphereGeometry(sectors, stacks);
        setupBuffers();
        std::cout << "Sphere created successfully with " << indexCount << " indices" << std::endl;
    }

    ~Sphere() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    void setPosition(const glm::vec3& pos) { position = pos; }
    void setColor(const glm::vec3& col) { color = col; }
    void setRadius(float r) { radius = r; }
    
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getColor() const { return color; }
    float getRadius() const { return radius; }

    void render(unsigned int shaderProgram) {
        glBindVertexArray(VAO);
        
        // Set model matrix based on position and radius
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(radius));
        
        // Set uniforms with error checking
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
        
        if (modelLoc != -1) {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        } else {
            std::cout << "Error: Could not find 'model' uniform" << std::endl;
        }
        
        if (colorLoc != -1) {
            glUniform3f(colorLoc, color.r, color.g, color.b);
        } else {
            std::cout << "Error: Could not find 'objectColor' uniform" << std::endl;
        }
        
        // Draw the sphere
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        
        // Check for OpenGL errors
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cout << "OpenGL error: " << err << std::endl;
        }
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
                
                // Normal (same as position for unit sphere)
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
    }

    void setupBuffers() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        std::cout << "Generated VAO: " << VAO << ", VBO: " << VBO << ", EBO: " << EBO << std::endl;
        
        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        std::cout << "Buffer setup complete. Vertices: " << vertices.size() << ", Indices: " << indices.size() << std::endl;
    }
};

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
    GLFWwindow* window = glfwCreateWindow(800, 600, "Solar System - Multiple Spheres", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // Configure OpenGL
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, 800, 600);
    
    // Create multiple spheres with different properties
    std::vector<Sphere> spheres;
    
    // Sun (large, orange)
    spheres.emplace_back(1.0f, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.6f, 0.2f));
    
    // Earth (medium, blue)
    spheres.emplace_back(0.3f, glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(0.2f, 0.5f, 1.0f));
    
    // Mars (small, red)
    spheres.emplace_back(0.2f, glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.3f, 0.3f));
    
    // Jupiter (large, orange-brown)
    spheres.emplace_back(0.8f, glm::vec3(-4.0f, 0.0f, 0.0f), glm::vec3(0.8f, 0.5f, 0.3f));
    
    // Saturn (medium, yellow)
    spheres.emplace_back(0.6f, glm::vec3(-7.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.9f, 0.6f));
    
    std::cout << "Created " << spheres.size() << " spheres" << std::endl;
    
    // Compile shaders
    unsigned int vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = createShaderProgram(vertexShader, fragmentShader);
    
    // Check if shader program was created successfully
    if (shaderProgram == 0) {
        std::cout << "Failed to create shader program!" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    std::cout << "Shader program created successfully: " << shaderProgram << std::endl;
    
    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Clear buffers
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Use shader program
        glUseProgram(shaderProgram);
        
        // Set view and projection matrices
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 15.0f),  // Camera position - moved closer
            glm::vec3(0.0f, 0.0f, 0.0f),   // Look at point
            glm::vec3(0.0f, 1.0f, 0.0f)    // Up vector
        );
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        
        // Light properties
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 2.0f, 2.0f, 2.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), 0.0f, 0.0f, 15.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
        
        // Render all spheres
        for (auto& sphere : spheres) {
            sphere.render(shaderProgram);
        }
        
        // Debug output (only print once)
        static bool debugPrinted = false;
        if (!debugPrinted) {
            std::cout << "Rendering " << spheres.size() << " spheres" << std::endl;
            debugPrinted = true;
        }
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Clean up
    glDeleteProgram(shaderProgram);
    
    glfwTerminate();
    return 0;
}
