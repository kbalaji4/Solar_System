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
    // Calculate gravitational forces between all spheres
    if (config.enableGravity) {
        calculateGravitationalForces(spheres, config);
    }
    
    for (auto& sphere : spheres) {
        // Only apply Earth gravity if gravitational attraction is disabled
        if (!config.enableGravity) {
            sphere.velocityY -= config.gravity * config.deltaTime;
        }
        
        // Update position using 3D velocity
        sphere.x += sphere.velocityX * config.deltaTime;
        sphere.y += sphere.velocityY * config.deltaTime;
        sphere.z += sphere.velocityZ * config.deltaTime;
        
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
        
        // Keep spheres within screen bounds (X and Z axes)
        float leftBoundary = -3.0f;
        float rightBoundary = 3.0f;
        float frontBoundary = -3.0f;
        float backBoundary = 3.0f;
        
        // X-axis boundaries
        if (sphere.x - sphere.radius < leftBoundary) {
            sphere.x = leftBoundary + sphere.radius;
            sphere.velocityX = -sphere.velocityX * config.bounceDamping;  // Bounce off left wall
        }
        if (sphere.x + sphere.radius > rightBoundary) {
            sphere.x = rightBoundary - sphere.radius;
            sphere.velocityX = -sphere.velocityX * config.bounceDamping;  // Bounce off right wall
        }
        
        // Z-axis boundaries
        if (sphere.z - sphere.radius < frontBoundary) {
            sphere.z = frontBoundary + sphere.radius;
            sphere.velocityZ = -sphere.velocityZ * config.bounceDamping;  // Bounce off front wall
        }
        if (sphere.z + sphere.radius > backBoundary) {
            sphere.z = backBoundary - sphere.radius;
            sphere.velocityZ = -sphere.velocityZ * config.bounceDamping;  // Bounce off back wall
        }
    }
}

void Sphere::calculateGravitationalForces(std::vector<SpherePhysics>& spheres, const PhysicsConfig& config) {
    for (size_t i = 0; i < spheres.size(); ++i) {
        for (size_t j = i + 1; j < spheres.size(); ++j) {
            auto& sphere1 = spheres[i];
            auto& sphere2 = spheres[j];
            
            // Calculate direction vector (matching your equation)
            float dx = sphere2.x - sphere1.x;
            float dy = sphere2.y - sphere1.y;
            float dz = sphere2.z - sphere1.z;
            float distance = sqrt(dx * dx + dy * dy + dz * dz);
            
            if (distance > 0) {
                // Calculate unit direction vector
                std::vector<float> direction = {dx / distance, dy / distance, dz / distance};
                
                // Scale distance for force calculation
                distance *= config.distanceScale;
                
                // Calculate gravitational force: F = G * m1 * m2 / r²
                double Gforce = (config.gravitationalConstant * sphere1.mass * sphere2.mass) / (distance * distance);
                
                // Calculate acceleration: a = F/m
                float acc1 = Gforce / sphere1.mass;
                std::vector<float> acc = {direction[0] * acc1, direction[1] * acc1, direction[2] * acc1};
                
                // Apply acceleration to velocities (v = v0 + a*t)
                sphere1.velocityX += acc[0] * config.deltaTime;
                sphere1.velocityY += acc[1] * config.deltaTime;
                sphere1.velocityZ += acc[2] * config.deltaTime;
                
                // Apply opposite force to sphere2
                sphere2.velocityX -= acc[0] * config.deltaTime;
                sphere2.velocityY -= acc[1] * config.deltaTime;
                sphere2.velocityZ -= acc[2] * config.deltaTime;
                
                // Limit maximum velocities to prevent runaway acceleration
                float maxVelocity = 10.0f;
                sphere1.velocityX = std::max(-maxVelocity, std::min(maxVelocity, sphere1.velocityX));
                sphere1.velocityY = std::max(-maxVelocity, std::min(maxVelocity, sphere1.velocityY));
                sphere1.velocityZ = std::max(-maxVelocity, std::min(maxVelocity, sphere1.velocityZ));
                
                sphere2.velocityX = std::max(-maxVelocity, std::min(maxVelocity, sphere2.velocityX));
                sphere2.velocityY = std::max(-maxVelocity, std::min(maxVelocity, sphere2.velocityY));
                sphere2.velocityZ = std::max(-maxVelocity, std::min(maxVelocity, sphere2.velocityZ));
                
                // Debug output (more frequently)
                static int debugCounter = 0;
                if (debugCounter++ % 60 == 0) {  // Every second at 60fps
                    std::cout << "Distance: " << distance/config.distanceScale << " units, Force: " << Gforce << "N" << std::endl;
                    std::cout << "Acceleration: (" << acc[0] << ", " << acc[1] << ", " << acc[2] << ")" << std::endl;
                    std::cout << "Sphere1 vel: (" << sphere1.velocityX << ", " << sphere1.velocityY << ", " << sphere1.velocityZ << ")" << std::endl;
                    std::cout << "Sphere2 vel: (" << sphere2.velocityX << ", " << sphere2.velocityY << ", " << sphere2.velocityZ << ")" << std::endl;
                }
            }
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
