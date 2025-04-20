#include <SFML/Graphics.hpp>
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
    const float maxZoom = 1000.0f;   // Maximum zoom out (increased for larger system)
    const float zoomSpeed = 2.0f;    // How quickly zoom changes

    // Clock for tracking time between frames
    sf::Clock clock;

    // Create game objects - main planet in the center (pinned in place)
    Planet planet(sf::Vector2f(400.f, 300.f), 500.f, 1000.f, sf::Color::Blue);
    // Set zero velocity to ensure it stays in place
    planet.setVelocity(sf::Vector2f(0.f, 0.f));

    // Create a second planet - position it much farther away
    Planet planet2(sf::Vector2f(15400.f, 300.f), 200.f, 500.f, sf::Color::Red);

    // Calculate proper orbital velocity for circular orbit
    float distance = 15000.0f; // Distance between planets (15400-400)
    // Using Kepler's laws: v = sqrt(G*M/r)
    float orbitSpeed = std::sqrt(100000.0f * planet.getMass() / distance);
    // Setting velocity perpendicular to the radial direction
    planet2.setVelocity(sf::Vector2f(0.f, orbitSpeed));

    // Calculate position on the first planet's edge - start at the top
    sf::Vector2f planetPos = planet.getPosition();
    float planetRadius = planet.getRadius();
    float rocketSize = 15.0f; // Approximate rocket size

    // Direction vector pointing from planet center to the top (0, -1)
    sf::Vector2f direction(0, -1);
    sf::Vector2f rocketPos = planetPos + direction * (planetRadius + rocketSize);

    // Create rocket at calculated position with a mass of 1.0 units
    Rocket rocket(rocketPos, sf::Vector2f(0.f, 0.f), sf::Color::White, 1.0f);

    // Set up gravity simulator
    GravitySimulator gravitySimulator;
    gravitySimulator.addPlanet(&planet);
    gravitySimulator.addPlanet(&planet2);
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
                    else if (keyEvent->code == sf::Keyboard::Key::P)
                    {
                        // Toggle planet gravity simulation with 'P' key
                        static bool planetGravity = true;
                        planetGravity = !planetGravity;
                        gravitySimulator.setSimulatePlanetGravity(planetGravity);
                    }
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

        // Add camera controls
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z)) {
            // Zoom out to see entire system
            targetZoom = 800.0f;
            // Center between planets
            gameView.setCenter((planet.getPosition() + planet2.getPosition()) / 2.0f);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X)) {
            // Follow the rocket - calculate distances manually
            float dist1 = std::sqrt(
                std::pow(rocket.getPosition().x - planet.getPosition().x, 2) +
                std::pow(rocket.getPosition().y - planet.getPosition().y, 2)
            );
            float dist2 = std::sqrt(
                std::pow(rocket.getPosition().x - planet2.getPosition().x, 2) +
                std::pow(rocket.getPosition().y - planet2.getPosition().y, 2)
            );
            targetZoom = minZoom + (std::min(dist1, dist2) - (planet.getRadius() + 15.0f)) / 100.0f;
            gameView.setCenter(rocket.getPosition());
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C)) {
            // Follow planet 2
            targetZoom = 100.0f;
            gameView.setCenter(planet2.getPosition());
        }

        // Update simulation
        gravitySimulator.update(deltaTime);
        planet.update(deltaTime);
        planet2.update(deltaTime);
        rocket.update(deltaTime);

        // Calculate distance from rocket to closest planet for zoom
        sf::Vector2f rocketToPlanet = planet.getPosition() - rocket.getPosition();
        sf::Vector2f rocketToPlanet2 = planet2.getPosition() - rocket.getPosition();
        float distance1 = std::sqrt(rocketToPlanet.x * rocketToPlanet.x + rocketToPlanet.y * rocketToPlanet.y);
        float distance2 = std::sqrt(rocketToPlanet2.x * rocketToPlanet2.x + rocketToPlanet2.y * rocketToPlanet2.y);

        // Use closest planet for zoom calculation (if not manually zooming)
        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z) &&
            !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X) &&
            !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C)) {
            float closest = std::min(distance1, distance2);
            targetZoom = minZoom + (closest - (planet.getRadius() + 15.0f)) / 100.0f;
            targetZoom = std::max(minZoom, std::min(targetZoom, maxZoom));

            // Update view center to follow rocket
            gameView.setCenter(rocket.getPosition());
        }

        // Smoothly interpolate current zoom to target zoom
        zoomLevel += (targetZoom - zoomLevel) * deltaTime * zoomSpeed;

        // Set view size based on zoom level
        gameView.setSize(sf::Vector2f(800.f * zoomLevel, 600.f * zoomLevel));

        // Apply the view before drawing
        window.setView(gameView);

        // Clear window with black background
        window.clear(sf::Color::Black);

        // Draw orbit paths
        planet.drawOrbitPath(window, gravitySimulator.getPlanets());
        planet2.drawOrbitPath(window, gravitySimulator.getPlanets());

        // Draw the rocket trajectory
        rocket.drawTrajectory(window, gravitySimulator.getPlanets());

        // Draw objects
        planet.draw(window);
        planet2.draw(window);
        rocket.drawWithConstantSize(window, zoomLevel);

        // Draw velocity vectors
        planet.drawVelocityVector(window, 5.0f);
        planet2.drawVelocityVector(window, 5.0f);
        rocket.drawVelocityVector(window, 2.0f);

        // Display what was drawn
        window.display();
    }

    return 0;
}