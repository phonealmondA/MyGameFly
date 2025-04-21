#include <SFML/Graphics.hpp>
#include "Planet.h"
#include "Rocket.h"
#include "GravitySimulator.h"
#include "Button.h"
#include <memory>
#include <vector>
#include <cstdint> // For uint8_t
#include <sstream>
#include <iomanip>
#include <limits>
#include <iostream> // Add this for std::cerr

#ifdef _DEBUG
#pragma comment(lib, "sfml-graphics-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#pragma comment(lib, "sfml-system-d.lib")
#else
#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-window.lib")
#pragma comment(lib, "sfml-system.lib")
#endif

// Define a simple TextPanel class to display information
class TextPanel {
private:
    sf::Text text;
    sf::RectangleShape background;

public:
    // Modified constructor to handle SFML 3.0 requirements
    // TextPanel constructor correction for setPosition
    TextPanel(const sf::Font& font, unsigned int characterSize, sf::Vector2f position,
        sf::Vector2f size, sf::Color bgColor = sf::Color(0, 0, 0, 180))
        : text(font) { // Initialize text with font
        // Setup background
        background.setPosition(position); // This is already using Vector2f, so it's correct
        background.setSize(size);
        background.setFillColor(bgColor);
        background.setOutlineColor(sf::Color::White);
        background.setOutlineThickness(1.0f);

        // Setup text
        text.setCharacterSize(characterSize);
        text.setFillColor(sf::Color::White);
        // In the TextPanel constructor:
        text.setPosition(sf::Vector2f(position.x + 5.f, position.y + 5.f));
    }
    void setText(const std::string& str) {
        text.setString(str);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(background);
        window.draw(text);
    }
};

// Helper functions to calculate orbital parameters
float calculateApoapsis(sf::Vector2f pos, sf::Vector2f vel, float planetMass, float G) {
    // Calculate specific orbital energy
    float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
    float distance = std::sqrt(pos.x * pos.x + pos.y * pos.y);
    float energy = 0.5f * speed * speed - G * planetMass / distance;

    // Calculate semi-major axis
    float semiMajor = -G * planetMass / (2 * energy);

    // If energy is positive, orbit is hyperbolic (no apoapsis)
    if (energy >= 0) return -1.0f;

    // Calculate eccentricity vector
    sf::Vector2f eVec;
    float vSquared = speed * speed;
    eVec.x = (vSquared * pos.x - (pos.x * vel.x + pos.y * vel.y) * vel.x) / (G * planetMass) - pos.x / distance;
    eVec.y = (vSquared * pos.y - (pos.x * vel.x + pos.y * vel.y) * vel.y) / (G * planetMass) - pos.y / distance;
    float ecc = std::sqrt(eVec.x * eVec.x + eVec.y * eVec.y);

    // Calculate apoapsis
    return semiMajor * (1 + ecc);
}

float calculatePeriapsis(sf::Vector2f pos, sf::Vector2f vel, float planetMass, float G) {
    // Calculate specific orbital energy
    float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
    float distance = std::sqrt(pos.x * pos.x + pos.y * pos.y);
    float energy = 0.5f * speed * speed - G * planetMass / distance;

    // Calculate semi-major axis
    float semiMajor = -G * planetMass / (2 * energy);

    // If energy is positive, orbit is hyperbolic
    if (energy >= 0) return -1.0f;

    // Calculate eccentricity vector
    sf::Vector2f eVec;
    float vSquared = speed * speed;
    eVec.x = (vSquared * pos.x - (pos.x * vel.x + pos.y * vel.y) * vel.x) / (G * planetMass) - pos.x / distance;
    eVec.y = (vSquared * pos.y - (pos.x * vel.x + pos.y * vel.y) * vel.y) / (G * planetMass) - pos.y / distance;
    float ecc = std::sqrt(eVec.x * eVec.x + eVec.y * eVec.y);

    // Calculate periapsis
    return semiMajor * (1 - ecc);
}

int main()
{
    // Create a window with increased size 1280x720
    sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), "Rocket Simulator");

    // Create a view for the camera with initial zoom level
    sf::View gameView(sf::Vector2f(640.f, 360.f), sf::Vector2f(1280.f, 720.f));
    float zoomLevel = 1.0f;
    float targetZoom = 1.0f;
    const float minZoom = 1.0f;      // Maximum zoom in (closest to planet)
    const float maxZoom = 1000.0f;   // Maximum zoom out (increased for larger system)
    const float zoomSpeed = 1.0f;  // Increase from 2.0f for faster zoom

    // Load a font for the buttons
    sf::Font font;
    // SFML 3.0 still uses openFromFile but now takes a std::filesystem::path
    if (!font.openFromFile("arial.ttf")) {
        // Try alternative paths
        if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
            std::cerr << "Warning: Could not load font file. Text won't display correctly." << std::endl;
        }
    }

    // Create UI view (fixed, doesn't zoom or move with game world)
    sf::View uiView(sf::Vector2f(640.f, 360.f), sf::Vector2f(1280.f, 720.f));

    // Create text panels for displaying information
    TextPanel rocketInfoPanel(font, 12, sf::Vector2f(10, 10), sf::Vector2f(250, 150));
    TextPanel planetInfoPanel(font, 12, sf::Vector2f(10, 170), sf::Vector2f(250, 120));
    TextPanel orbitInfoPanel(font, 12, sf::Vector2f(10, 300), sf::Vector2f(250, 100));

    // Gravitational constant - same as in GravitySimulator
    const float G = 100000.0f;

    // Create buttons
    std::vector<Button> buttons;

    /* Zoom in button
    buttons.emplace_back(
        sf::Vector2f(20.f, 20.f), sf::Vector2f(80.f, 30.f),
        "Zoom In", font,
        [&]() {
            targetZoom = std::max(minZoom, targetZoom / 1.0f); // More dramatic zoom
        }
    );

    // Zoom out button
    buttons.emplace_back(
        sf::Vector2f(20.f, 60.f), sf::Vector2f(80.f, 30.f),
        "Zoom Out", font,
        [&]() {
            targetZoom = std::min(maxZoom, targetZoom * 1.0f); // More dramatic zoom
        }
    );

    // Reset view button
    buttons.emplace_back(
        sf::Vector2f(20.f, 100.f), sf::Vector2f(80.f, 30.f),
        "Reset View", font,
        [&]() {
            targetZoom = 1.0f;
            gameView.setCenter(sf::Vector2f(400.f, 300.f));
        }
    );
    */
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
    Rocket rocket(rocketPos, sf::Vector2f(0.f, 0.f), sf::Color::White, 0.1f);

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

            // Handle window resize events
            if (event->is<sf::Event::Resized>())
            {
                const auto* resizeEvent = event->getIf<sf::Event::Resized>();
                if (resizeEvent)
                {
                    // In SFML 3.0, we need to access the size property
                    float aspectRatio = static_cast<float>(resizeEvent->size.x) / static_cast<float>(resizeEvent->size.y);
                    gameView.setSize(sf::Vector2f(
                        resizeEvent->size.y * aspectRatio * zoomLevel,
                        resizeEvent->size.y * zoomLevel
                    ));

                    // Update UI view to match new window size
                    uiView.setSize(sf::Vector2f(
                        static_cast<float>(resizeEvent->size.x),
                        static_cast<float>(resizeEvent->size.y)
                    ));
                    uiView.setCenter(sf::Vector2f(
                        static_cast<float>(resizeEvent->size.x) / 2.0f,
                        static_cast<float>(resizeEvent->size.y) / 2.0f
                    ));

                    window.setView(gameView);
                }
            }


            // Handle mouse button press events for buttons
            if (event->is<sf::Event::MouseButtonPressed>())
            {
                const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>();
                if (mouseEvent && mouseEvent->button == sf::Mouse::Button::Left)
                {
                    // Important: temporarily set UI view before checking button clicks
                    window.setView(uiView);

                    // Get current mouse position
                    sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
                    sf::Vector2f mousePos = window.mapPixelToCoords(mousePosition, uiView);

                    // Check each button
                    for (auto& button : buttons)
                    {
                        if (button.contains(mousePos))
                        {
                            button.handleClick();
                            // Optional: Add debug output
                            // std::cout << "Button clicked!" << std::endl;
                        }
                    }

                    // Reset back to game view
                    window.setView(gameView);
                }
            }

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

        // In main.cpp, modify the Z key handling code
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z)) {
            // Gradually increase zoom to see more of the system
            targetZoom = std::min(80.0f, targetZoom * 1.05f); // Increase by 5% each frame
            // Center between planets
            //changed from: planet.getPosition() + planet2.getPosition()) / 2.0f : to focus rocket, not senter between planets. 
            gameView.setCenter(rocket.getPosition());
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
        gameView.setSize(sf::Vector2f(1280.f * zoomLevel, 720.f * zoomLevel));

        // Apply the view before drawing game objects
        window.setView(gameView);

        // Clear window with black background
        window.clear(sf::Color::Black);

        // Draw orbit paths
        planet.drawOrbitPath(window, gravitySimulator.getPlanets());
        planet2.drawOrbitPath(window, gravitySimulator.getPlanets());

        // Draw the rocket trajectory
        rocket.drawTrajectory(window, gravitySimulator.getPlanets(), 0.5f, 200, false);

        // Draw objects
        planet.draw(window);
        planet2.draw(window);
        rocket.drawWithConstantSize(window, zoomLevel);

        // Draw velocity vectors
        planet.drawVelocityVector(window, 5.0f);
        planet2.drawVelocityVector(window, 5.0f);
        rocket.drawVelocityVector(window, 2.0f);

        // Add this line to draw gravity force vectors
        rocket.drawGravityForceVectors(window, gravitySimulator.getPlanets(), 1000.0f);

        // Update info panels with current data

        // 1. Rocket information
        {
            std::stringstream ss;
            float speed = std::sqrt(rocket.getVelocity().x * rocket.getVelocity().x +
                rocket.getVelocity().y * rocket.getVelocity().y);

            ss << "ROCKET INFO\n"
                << "Mass: " << rocket.getMass() << " units\n"
                << "Speed: " << std::fixed << std::setprecision(1) << speed << " units/s\n"
                << "Velocity: (" << std::setprecision(1) << rocket.getVelocity().x << ", "
                << rocket.getVelocity().y << ")\n";

            // Calculate total gravity force from all planets
            float totalForce = 0.0f;

            for (const auto& planetPtr : gravitySimulator.getPlanets()) {
                sf::Vector2f direction = planetPtr->getPosition() - rocket.getPosition();
                float dist = std::sqrt(direction.x * direction.x + direction.y * direction.y);

                float forceMagnitude = G * planetPtr->getMass() * rocket.getMass() / (dist * dist);
                totalForce += forceMagnitude;

                // Label planets by color for clarity
                std::string planetName = (planetPtr == &planet) ? "Blue Planet" : "Red Planet";
                ss << "Force from " << planetName
                    << ": " << std::setprecision(0) << forceMagnitude << " units\n";
            }

            ss << "Total gravity force: " << std::setprecision(0) << totalForce << " units";
            rocketInfoPanel.setText(ss.str());
        }

        // 2. Closest planet information
        {
            std::stringstream ss;
            Planet* closestPlanet = nullptr;
            float closestDistance = std::numeric_limits<float>::max();

            for (const auto& planetPtr : gravitySimulator.getPlanets()) {
                sf::Vector2f direction = planetPtr->getPosition() - rocket.getPosition();
                float dist = std::sqrt(direction.x * direction.x + direction.y * direction.y);

                if (dist < closestDistance) {
                    closestDistance = dist;
                    closestPlanet = planetPtr;
                }
            }

            if (closestPlanet) {
                std::string planetName = (closestPlanet == &planet) ? "Blue Planet" : "Red Planet";
                float speed = std::sqrt(closestPlanet->getVelocity().x * closestPlanet->getVelocity().x +
                    closestPlanet->getVelocity().y * closestPlanet->getVelocity().y);

                ss << "NEAREST PLANET: " << planetName << "\n"
                    << "Distance: " << std::fixed << std::setprecision(0) << closestDistance << " units\n"
                    << "Mass: " << closestPlanet->getMass() << " units\n"
                    << "Radius: " << closestPlanet->getRadius() << " units\n"
                    << "Speed: " << std::setprecision(1) << speed << " units/s\n"
                    << "Surface gravity: " << std::setprecision(2)
                    << G * closestPlanet->getMass() / (closestPlanet->getRadius() * closestPlanet->getRadius()) << " units/s²";

                planetInfoPanel.setText(ss.str());
            }
        }

        // 3. Orbit information
        {
            std::stringstream ss;
            Planet* primaryBody = nullptr;
            float strongestGravity = 0.0f;

            // Find the primary gravitational body (usually the closest one)
            for (const auto& planetPtr : gravitySimulator.getPlanets()) {
                sf::Vector2f direction = planetPtr->getPosition() - rocket.getPosition();
                float dist = std::sqrt(direction.x * direction.x + direction.y * direction.y);

                float gravityStrength = G * planetPtr->getMass() / (dist * dist);
                if (gravityStrength > strongestGravity) {
                    strongestGravity = gravityStrength;
                    primaryBody = planetPtr;
                }
            }

            if (primaryBody) {
                // Calculate relative position and velocity to primary body
                sf::Vector2f relPos = rocket.getPosition() - primaryBody->getPosition();
                sf::Vector2f relVel = rocket.getVelocity() - primaryBody->getVelocity();

                // Calculate orbit parameters
                float apoapsis = calculateApoapsis(relPos, relVel, primaryBody->getMass(), G);
                float periapsis = calculatePeriapsis(relPos, relVel, primaryBody->getMass(), G);

                // Current distance
                float distance = std::sqrt(relPos.x * relPos.x + relPos.y * relPos.y);

                // Check if orbit is valid (not hyperbolic)
                if (periapsis > 0 && apoapsis > 0) {
                    ss << "ORBIT INFO\n"
                        << "Primary: " << (primaryBody == &planet ? "Blue Planet" : "Red Planet") << "\n"
                        << "Periapsis: " << std::fixed << std::setprecision(0) << periapsis << " units\n"
                        << "Apoapsis: " << std::setprecision(0) << apoapsis << " units\n"
                        << "Current dist: " << std::setprecision(0) << distance << " units";
                }
                else {
                    ss << "ORBIT INFO\n"
                        << "Primary: " << (primaryBody == &planet ? "Blue Planet" : "Red Planet") << "\n"
                        << "Orbit: Escape trajectory\n"
                        << "Current dist: " << std::fixed << std::setprecision(0) << distance << " units";
                }

                orbitInfoPanel.setText(ss.str());
            }
        }

        // Switch to UI view for text panels
        window.setView(uiView);

        // Draw info panels
        rocketInfoPanel.draw(window);
        planetInfoPanel.draw(window);
        orbitInfoPanel.draw(window);

        // Update and draw buttons
        sf::Vector2f mousePos = window.mapPixelToCoords(
            sf::Mouse::getPosition(window), uiView);
        for (auto& button : buttons) {
            button.update(mousePos);
            button.draw(window);
        }

        // Display what was drawn
        window.display();
    }

    return 0;
}