#pragma once

namespace GameConstants {
    // Gravitational constants
    constexpr float G = 100.0f;  // Gravitational constant

    // Planet configuration
    constexpr float MAIN_PLANET_MASS = 2500000.0f;
    constexpr float SECONDARY_PLANET_MASS = 1600000.0f;
    constexpr float MAIN_PLANET_RADIUS = 1000.0f;
    constexpr float SECONDARY_PLANET_RADIUS = 500.0f;

    // Planet positions
    constexpr float MAIN_PLANET_X = 400.0f;
    constexpr float MAIN_PLANET_Y = 300.0f;
    constexpr float SECONDARY_PLANET_X = 20000.0f;
    constexpr float SECONDARY_PLANET_Y = 300.0f;

    // Rocket parameters
    constexpr float ROCKET_MASS = 1.0f;
    constexpr float ROCKET_SIZE = 15.0f;

    // Visualization settings
    constexpr float GRAVITY_VECTOR_SCALE = 100.0f;
    constexpr float VELOCITY_VECTOR_SCALE = 0.01f;

    // Trajectory calculation settings
    constexpr float TRAJECTORY_TIME_STEP = 0.05f;
    constexpr int TRAJECTORY_STEPS = 5000;
    constexpr float TRAJECTORY_COLLISION_RADIUS = 10.0f;
    // Vehicle physics
    constexpr float FRICTION = 0.0098f;  // Friction coefficient for surface movement
    constexpr float TRANSFORM_DISTANCE = 30.0f;  // Distance for vehicle transformation
    constexpr float ADAPTIVE_TIMESTEP_THRESHOLD = 10.0f;  // Threshold for adaptive timestep
    constexpr float CAR_WHEEL_RADIUS = 5.0f;  // Radius of car wheels
    constexpr float CAR_BODY_WIDTH = 30.0f;  // Width of car body
    constexpr float CAR_BODY_HEIGHT = 15.0f;  // Height of car body
    // Engine parameters
    constexpr float BASE_THRUST_MULTIPLIER = 100000000.0f;
    constexpr float ENGINE_THRUST_POWER = G * BASE_THRUST_MULTIPLIER;

    // Vehicle transformation parameters
    constexpr float TRANSFORM_VELOCITY_FACTOR = 0.1f;  // Velocity reduction when transforming from rocket to car


}