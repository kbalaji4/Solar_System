# Solar System Gravitational Attraction

## Physics Features

### Gravitational Attraction
- **Realistic Physics**: Uses F = GMm/d² gravitational force equation
- **3D Movement**: Spheres can move in all three dimensions
- **Mass-based**: Each sphere has a realistic mass (moon mass)
- **Distance Scaling**: Realistic distance scaling (×1000) for proper force calculation
- **Toggle On/Off**: Easy to enable/disable gravitational attraction

### Physics Implementation
- **Gravitational Constant**: G = 6.67430 × 10⁻¹¹ m³/kg/s²
- **Distance Calculation**: 3D distance between sphere centers
- **Force Application**: F = ma applied to velocities
- **Collision Avoidance**: Prevents division by zero for very small distances

## Current Setup

### Two Moon-Mass Spheres
- **Red Sphere**: Moon mass (7.342 × 10²² kg) at (-2, 0, 0)
- **Blue Sphere**: Moon mass (7.342 × 10²² kg) at (2, 0, 0)
- **Initial Distance**: 4 units × 1000 = 4000 meters
- **Initial Velocity**: Both spheres start at rest

### Physics Configuration
```cpp
PhysicsConfig {
    enableGravity = true;           // Gravitational attraction
    gravitationalConstant = 6.67430e-11f;  // G constant
    distanceScale = 1000.0f;        // Distance scaling
    enableBounce = true;            // Boundary bouncing
    gravity = 9.8f;                 // Earth gravity for boundaries
}
```

## Controls

### Physics Controls
- **G** - Toggle gravitational attraction on/off
- **B** - Toggle bounce physics on/off
- **+** - Increase bounce damping (more bouncy)
- **-** - Decrease bounce damping (less bouncy)
- **ESC** - Exit the program

### Visual Feedback
- Console shows gravitational attraction toggle
- Console shows distance and force every 5 seconds
- Console shows bounce/wrap events

## Physics Behavior

### With Gravitational Attraction (Default)
- Spheres attract each other with realistic force
- Force increases as spheres get closer
- Spheres accelerate toward each other
- Eventually collide or reach minimum distance
- Earth gravity still affects Y-axis movement

### Without Gravitational Attraction
- Spheres fall with only Earth gravity
- No mutual attraction
- Standard bounce/wrap behavior

## Technical Details

### Force Calculation
```cpp
// Distance between spheres (scaled)
float dx = (sphere2.x - sphere1.x) * distanceScale;
float dy = (sphere2.y - sphere1.y) * distanceScale;
float dz = (sphere2.z - sphere1.z) * distanceScale;
float distance = sqrt(dx*dx + dy*dy + dz*dz);

// Gravitational force: F = G * m1 * m2 / r²
float force = G * sphere1.mass * sphere2.mass / (distance * distance);

// Force components
float forceX = force * dx / distance;
float forceY = force * dy / distance;
float forceZ = force * dz / distance;
```

### Velocity Updates
```cpp
// Apply forces to velocities (F = ma, so a = F/m)
sphere1.velocityX += forceX / sphere1.mass * deltaTime;
sphere1.velocityY += forceY / sphere1.mass * deltaTime;
sphere1.velocityZ += forceZ / sphere1.mass * deltaTime;
```

## Example Usage

1. **Start simulation** - Two moon-mass spheres will attract each other
2. **Press G** - Disable gravitational attraction (spheres fall independently)
3. **Press G again** - Re-enable gravitational attraction
4. **Press B** - Toggle between bounce and wrap modes
5. **Watch console** - See distance and force measurements

## Future Enhancements

- Add more celestial bodies (planets, stars)
- Implement orbital mechanics
- Add initial velocities for orbital motion
- Scale masses to realistic solar system values
- Add collision detection and merging
