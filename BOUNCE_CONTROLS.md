# Bounce Physics Controls

## Physics Features

### Bounce Physics
- **Realistic Bouncing**: Spheres now bounce off the bottom boundary instead of wrapping around
- **Energy Loss**: Each bounce loses some energy (damping factor)
- **Toggle On/Off**: Easy to switch between bounce and wrap-around modes

### Physics Configuration
- **Bounce Damping**: Controls how much energy is lost on each bounce
  - `0.0` = No bounce (sphere stops)
  - `1.0` = Perfect bounce (no energy loss)
  - `0.8` = Default (20% energy loss per bounce)

## Controls

### Physics Controls
- **B** - Toggle bounce physics on/off
- **+** - Increase bounce damping (more bouncy)
- **-** - Decrease bounce damping (less bouncy)
- **ESC** - Exit the program

### Visual Feedback
- Console output shows when bounce is toggled
- Console shows bounce damping value changes
- Console shows when spheres bounce or wrap

## Physics Behavior

### Bounce Mode (Default)
- Spheres fall with gravity (9.8 m/s²)
- When hitting bottom boundary, spheres bounce back up
- Each bounce reduces velocity by damping factor
- Eventually spheres settle at the bottom

### Wrap Mode
- Spheres fall with gravity
- When hitting bottom boundary, spheres teleport to top
- Velocity resets to 0
- Continuous falling cycle

## Example Usage

1. **Start the simulation** - Spheres will bounce by default
2. **Press B** - Toggle to wrap-around mode
3. **Press B again** - Return to bounce mode
4. **Press +** - Make bounces more energetic
5. **Press -** - Make bounces less energetic

## Technical Details

### Physics Implementation
- Uses realistic collision detection with sphere radius
- Proper boundary placement to prevent overlap
- Velocity reversal with damping for realistic bouncing
- Configurable physics parameters

### Configuration Structure
```cpp
struct PhysicsConfig {
    bool enableBounce = true;      // Toggle bounce/wrap
    float bounceDamping = 0.8f;    // Energy loss (0.0-1.0)
    float gravity = 9.8f;          // Gravity strength
    float deltaTime = 1.0f/60.0f;  // Physics timestep
    float bottomBoundary = -2.0f;  // Bottom boundary
    float topBoundary = 2.0f;      // Top boundary
};
```
