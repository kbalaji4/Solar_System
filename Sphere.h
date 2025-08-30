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
    float bounceDamping = 1.0f;  // Energy loss on bounce (0.0 = no bounce, 1.0 = perfect bounce)
    bool enableGravity = true;  // Toggle gravitational attraction
    float gravity = 9.8f;  // Earth gravity (for boundary bouncing)
    float deltaTime = 1.0f / 60.0f;
    float bottomBoundary = -2.0f;
    float topBoundary = 2.0f;
    float distanceScale = 100.0f;  // Reduced scale factor for manageable forces
    float gravitationalConstant = 6.67430e-11f;  // Real G constant in m³/kg/s²
};

// Physics structure for sphere properties
struct SpherePhysics {
    float x, y, z;
    float velocityX, velocityY, velocityZ;  // 3D velocity
    float radius;
    float mass;  // Mass of the sphere
    glm::vec3 color;
    
    SpherePhysics(float x, float y, float z, float vx, float vy, float vz, float r, float m, glm::vec3 c) 
        : x(x), y(y), z(z), velocityX(vx), velocityY(vy), velocityZ(vz), radius(r), mass(m), color(c) {}
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
    static void calculateGravitationalForces(std::vector<SpherePhysics>& spheres, const PhysicsConfig& config);
    static void renderAllSpheres(const std::vector<SpherePhysics>& spheres, Sphere* sphereGeometry, 
                                unsigned int shaderProgram, const glm::vec3& lightPos, 
                                const glm::vec3& viewPos, const glm::vec3& lightColor);
};

#endif // SPHERE_H
