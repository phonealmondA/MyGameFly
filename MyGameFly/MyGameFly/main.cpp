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

    // Clock for tracking time between frames
    sf::Clock clock;

    // Create game objects
    Planet planet(sf::Vector2f(400.f, 300.f), 50.f, 1000000.f, sf::Color::Blue);
    // Position rocket at top of planet
    Rocket rocket(sf::Vector2f(planet.getPosition().x,
        planet.getPosition().y - planet.getRadius() - 30.f),
        sf::Vector2f(0.f, 0.f), sf::Color::White);

    // Set up gravity simulator
    GravitySimulator gravitySimulator;
    gravitySimulator.addPlanet(&planet);
    gravitySimulator.addRocket(&rocket);

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
            rocket.rotate(-6.0f * deltaTime * 60.0f);  // Increased from 2.0f to 3.0f
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            rocket.rotate(6.0f * deltaTime * 60.0f);   // Increased from 2.0f to 3.0f

        // Update simulation
        gravitySimulator.update(deltaTime);
        planet.update(deltaTime);
        rocket.update(deltaTime);

        // Check for collision
        if (rocket.isColliding(planet)) {
            // Reset rocket position to top of planet
            rocket = Rocket(sf::Vector2f(planet.getPosition().x,
                planet.getPosition().y - planet.getRadius() - 30.f),
                sf::Vector2f(0.f, 0.f));

            // Clear old rocket and add new one to gravity simulator
            gravitySimulator.clearRockets();
            gravitySimulator.addRocket(&rocket);
        }

        // Clear window with black background
        window.clear(sf::Color::Black);

        // Draw the trajectory first so it appears behind other elements
        rocket.drawTrajectory(window, gravitySimulator.getPlanets());

        // Draw objects
        planet.draw(window);
        rocket.draw(window);
        rocket.drawVelocityVector(window, 2.0f);

        // Display what was drawn
        window.display();
    }

    return 0;
}