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
    Rocket rocket(sf::Vector2f(200.f, 300.f), sf::Vector2f(50.f, 0.f), sf::Color::White);

    // Set up gravity simulator
    GravitySimulator gravitySimulator;
    gravitySimulator.addPlanet(&planet);
    gravitySimulator.addRocket(&rocket);

    // Main game loop
    while (window.isOpen())
    {
        // Calculate delta time (limit to avoid physics issues if game freezes temporarily)
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
                    else if (keyEvent->code == sf::Keyboard::Key::Up)
                        rocket.applyThrust(10.0f);
                    else if (keyEvent->code == sf::Keyboard::Key::Left)
                        rocket.rotate(-5.0f);
                    else if (keyEvent->code == sf::Keyboard::Key::Right)
                        rocket.rotate(5.0f);
                }
            }
        }

        // Handle continuous key presses
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            rocket.applyThrust(1.0f);

        // Update simulation
        gravitySimulator.update(deltaTime);
        planet.update(deltaTime);
        rocket.update(deltaTime);

        // Clear window with black background
        window.clear(sf::Color::Black);

        // Draw objects
        planet.draw(window);
        rocket.draw(window);
        rocket.drawVelocityVector(window, 2.0f);

        // Display what was drawn
        window.display();
    }

    return 0;
}