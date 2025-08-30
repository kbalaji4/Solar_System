#ifndef SPHERE_H
#define SPHERE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>

// Physics configuration
struct PhysicsConfig {
    bool enableBounce = true;  // Toggle bounce on/off
    float bounceDamping = 0.8f;  // Energy loss on bounce (0.0 = no bounce, 1.0 = perfect bounce)
    float gravity = 9.8f;
    float deltaTime = 1.0f / 60.0f;
    float bottomBoundary = -2.0f;
    float topBoundary = 2.0f;
};

// Physics structure for sphere properties
struct SpherePhysics {
    float x, y, z;
    float velocityY;
    float radius;
    glm::vec3 color;
    
    SpherePhysics(float x, float y, float z, float velY, float r, glm::vec3 c) 
        : x(x), y(y), z(z), velocityY(velY), radius(r), color(c) {}
};

class Sphere {
private:
    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    float radius;
    
public:
    Sphere(float r = 1.0f);
    ~Sphere();
    
    void createSphere();
    void setupBuffers();
    void render(float x, float y, float z, float scale, unsigned int shaderProgram);
    
    // Physics methods
    static void updatePhysics(std::vector<SpherePhysics>& spheres, const PhysicsConfig& config);
    static void renderAllSpheres(const std::vector<SpherePhysics>& spheres, Sphere* sphereGeometry, 
                                unsigned int shaderProgram, const glm::vec3& lightPos, 
                                const glm::vec3& viewPos, const glm::vec3& lightColor);
};

#endif // SPHERE_H
