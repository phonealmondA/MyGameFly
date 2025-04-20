#include "D:/MyGameFly/SFML-3.0.0-windows-vc17-64-bit/SFML-3.0.0/include/SFML/Graphics.hpp"
#include "Planet.h"
#include "Rocket.h"
#include "GravitySimulator.h"
#include <memory>

#ifdef _DEBUG
#pragma comment(lib, "sfml-graphics-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#pragma comment(lib, "sfml-system-d.lib")
#else
#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-window.lib")
#pragma comment(lib, "sfml-system.lib")
#endif

int main()
{
    // Create a window with size 800x600
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Rocket Simulator");

    // Create a view for the camera with initial zoom level
    sf::View gameView(sf::Vector2f(400.f, 300.f), sf::Vector2f(800.f, 600.f));
    float zoomLevel = 1.0f;
    float targetZoom = 1.0f;
    const float minZoom = 1.0f;      // Maximum zoom in (closest to planet)
    const float maxZoom = 100.0f;    // Maximum zoom out (increased to see larger planet)
    const float zoomSpeed = 2.0f;    // How quickly zoom changes

    // Clock for tracking time between frames
    sf::Clock clock;

    // Create game objects - planet is now 600 times larger (30000 vs 50)
    // Create game objects with a much smaller planet
    Planet planet(sf::Vector2f(400.f, 300.f), 500.f, 1000.f, sf::Color::Blue);

    // Calculate position on the planet's edge - start at the top
    sf::Vector2f planetPos = planet.getPosition();
    float planetRadius = planet.getRadius();
    float rocketSize = 15.0f; // Approximate rocket size

    // Direction vector pointing from planet center to the top (0, -1)
    sf::Vector2f direction(0, -1);
    sf::Vector2f rocketPos = planetPos + direction * (planetRadius + rocketSize);

    // Create rocket at calculated position with a mass of 1000 units
    Rocket rocket(rocketPos, sf::Vector2f(0.f, 0.f), sf::Color::White, 1.0f);

    // Set up gravity simulator
    GravitySimulator gravitySimulator;
    gravitySimulator.addPlanet(&planet);
    gravitySimulator.addRocket(&rocket);

    // Give rocket access to planets for resting calculation
    rocket.setNearbyPlanets(gravitySimulator.getPlanets());

    // Main game loop
    while (window.isOpen())
    {
        // Calculate delta time
        float deltaTime = std::min(clock.restart().asSeconds(), 0.1f);

        // Check for events
        if (std::optional<sf::Event> event = window.pollEvent())
        {
            // Close the window when the close button is clicked
            if (event->is<sf::Event::Closed>())
                window.close();

            // Handle key events
            if (event->is<sf::Event::KeyPressed>())
            {
                const auto* keyEvent = event->getIf<sf::Event::KeyPressed>();
                if (keyEvent)
                {
                    if (keyEvent->code == sf::Keyboard::Key::Escape)
                        window.close();
                }
            }
        }

        // Handle continuous key presses for thrust level
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1))
            rocket.setThrustLevel(0.1f);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2))
            rocket.setThrustLevel(0.2f);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3))
            rocket.setThrustLevel(0.3f);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num4))
            rocket.setThrustLevel(0.4f);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num5))
            rocket.setThrustLevel(0.5f);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num6))
            rocket.setThrustLevel(0.6f);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num7))
            rocket.setThrustLevel(0.7f);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num8))
            rocket.setThrustLevel(0.8f);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num9))
            rocket.setThrustLevel(0.9f);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num0))
            rocket.setThrustLevel(0.0f);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Equal)) // = key
            rocket.setThrustLevel(1.0f);

        // Apply thrust and rotation
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            rocket.applyThrust(1.0f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            rocket.rotate(-6.0f * deltaTime * 60.0f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            rocket.rotate(6.0f * deltaTime * 60.0f);

        // Update simulation
        gravitySimulator.update(deltaTime);
        planet.update(deltaTime);
        rocket.update(deltaTime);

        // Calculate distance from rocket to planet and speed for zoom
        sf::Vector2f rocketToPlanet = planet.getPosition() - rocket.getPosition();
        float distance = std::sqrt(rocketToPlanet.x * rocketToPlanet.x + rocketToPlanet.y * rocketToPlanet.y);
        float rocketSpeed = std::sqrt(rocket.getVelocity().x * rocket.getVelocity().x +
            rocket.getVelocity().y * rocket.getVelocity().y);

        // Determine target zoom level based on distance and velocity
        targetZoom = minZoom + (distance - (planet.getRadius() + 15.0f)) / 100.0f;
        targetZoom = std::max(minZoom, std::min(targetZoom, maxZoom));

        // Smoothly interpolate current zoom to target zoom
        zoomLevel += (targetZoom - zoomLevel) * deltaTime * zoomSpeed;

        // Update view center to follow rocket
        gameView.setCenter(rocket.getPosition());

        // Set view size based on zoom level
        gameView.setSize(sf::Vector2f(800.f * zoomLevel, 600.f * zoomLevel));

        // Apply the view before drawing
        window.setView(gameView);

        // Clear window with black background
        window.clear(sf::Color::Black);

        // Draw the trajectory first so it appears behind other elements
        rocket.drawTrajectory(window, gravitySimulator.getPlanets());

        // Draw objects
        planet.draw(window);
        rocket.drawWithConstantSize(window, zoomLevel);
        rocket.drawVelocityVector(window, 2.0f);

        // Display what was drawn
        window.display();
    }

    return 0;
}