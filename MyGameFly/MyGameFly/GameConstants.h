#pragma once

namespace GameConstants {
    // Gravitational constants
    constexpr float G = 250.0f;  // Gravitational constant

    // Planet configuration
    constexpr float MAIN_PLANET_MASS = 1000.0f;
    constexpr float SECONDARY_PLANET_MASS = 500.0f;
    constexpr float MAIN_PLANET_RADIUS = 50.0f;
    constexpr float SECONDARY_PLANET_RADIUS = 30.0f;

    // Planet positions
    constexpr float MAIN_PLANET_X = 400.0f;
    constexpr float MAIN_PLANET_Y = 300.0f;
    constexpr float SECONDARY_PLANET_X = 1600.0f;
    constexpr float SECONDARY_PLANET_Y = 300.0f;

    // Rocket parameters
    constexpr float ROCKET_MASS = 1.0f;
    constexpr float ROCKET_SIZE = 15.0f;

    // Visualization settings
    constexpr float GRAVITY_VECTOR_SCALE = 100.0f;
    constexpr float VELOCITY_VECTOR_SCALE = 2.0f;

    // Trajectory calculation settings
    constexpr float TRAJECTORY_TIME_STEP = 0.05f;
    constexpr int TRAJECTORY_STEPS = 100;
    constexpr float TRAJECTORY_COLLISION_RADIUS = 10.0f;
}