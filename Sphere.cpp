#include "Sphere.h"

Sphere::Sphere(float r) : radius(r) {
    createSphere();
    setupBuffers();
}

Sphere::~Sphere() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Sphere::createSphere() {
    const int segments = 20;
    const int rings = 20;
    
    // Generate vertices
    for (int ring = 0; ring <= rings; ++ring) {
        float phi = glm::radians(180.0f * ring / rings);
        float y = radius * cos(phi);
        float ringRadius = radius * sin(phi);
        
        for (int segment = 0; segment <= segments; ++segment) {
            float theta = glm::radians(360.0f * segment / segments);
            float x = ringRadius * cos(theta);
            float z = ringRadius * sin(theta);
            
            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            
            // Normal
            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
        }
    }
    
    // Generate indices
    for (int ring = 0; ring < rings; ++ring) {
        for (int segment = 0; segment < segments; ++segment) {
            int first = ring * (segments + 1) + segment;
            int second = first + segments + 1;
            
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
    
    std::cout << "Generated " << vertices.size() / 6 << " vertices and " << indices.size() << " indices" << std::endl;
}

void Sphere::setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
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
    
    glBindVertexArray(0);
    std::cout << "Buffers setup complete" << std::endl;
}

void Sphere::render(float x, float y, float z, float scale, unsigned int shaderProgram) {
    glBindVertexArray(VAO);
    
    // Create model matrix for position and scale
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, z));
    model = glm::scale(model, glm::vec3(scale, scale, scale));
    
    // Pass model matrix to shader
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Sphere::updatePhysics(std::vector<SpherePhysics>& spheres, const PhysicsConfig& config) {
    for (auto& sphere : spheres) {
        // Apply gravity
        sphere.velocityY -= config.gravity * config.deltaTime;
        
        // Update position
        sphere.y += sphere.velocityY * config.deltaTime;
        
        // Check for bottom boundary
        if (sphere.y - sphere.radius < config.bottomBoundary) {
            if (config.enableBounce) {
                // Bounce physics
                sphere.y = config.bottomBoundary + sphere.radius;  // Place sphere at boundary
                sphere.velocityY = -sphere.velocityY * config.bounceDamping;  // Reverse velocity with damping
                std::cout << "Sphere bounced! Velocity: " << sphere.velocityY << std::endl;
            } else {
                // Wrap around (original behavior)
                sphere.y = config.topBoundary + sphere.radius;  // Wrap to top
                sphere.velocityY = 0.0f;  // Reset velocity
                std::cout << "Sphere wrapped to top! Position: (" << sphere.x << ", " << sphere.y << ")" << std::endl;
            }
        }
        
        // Check for top boundary (in case sphere somehow goes above)
        if (sphere.y + sphere.radius > config.topBoundary) {
            sphere.y = config.topBoundary - sphere.radius;
            sphere.velocityY = 0.0f;  // Stop upward motion
        }
    }
}

void Sphere::renderAllSpheres(const std::vector<SpherePhysics>& spheres, Sphere* sphereGeometry, 
                             unsigned int shaderProgram, const glm::vec3& lightPos, 
                             const glm::vec3& viewPos, const glm::vec3& lightColor) {
    // Set lighting uniforms
    glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), viewPos.x, viewPos.y, viewPos.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), lightColor.x, lightColor.y, lightColor.z);
    
    // Render all spheres
    for (const auto& spherePhysics : spheres) {
        // Set color for this sphere
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 
                   spherePhysics.color.x, spherePhysics.color.y, spherePhysics.color.z);
        
        // Render sphere at current physics position with its size
        sphereGeometry->render(spherePhysics.x, spherePhysics.y, spherePhysics.z, 
                              spherePhysics.radius, shaderProgram);
    }
}
