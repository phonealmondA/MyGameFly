#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include "Planet.h"
#include "Rocket.h"
#include "Car.h"
#include "VehicleManager.h"
#include "GravitySimulator.h"
#include "GameConstants.h"
#include "Button.h"
#include "NetworkManager.h"
#include "GameServer.h"
#include "GameClient.h"
#include "GameState.h"
#include "PlayerInput.h"
#include <memory>
#include <vector>
#include <cstdint> // For uint8_t
#include <sstream>
#include <iomanip>
#include <limits>
#include <iostream> // For std::cerr
enum class GameStateA {
    MENU,
    JOIN_MENU,
    SINGLE_PLAYER,
    MULTIPLAYER_HOST,
    MULTIPLAYER_CLIENT
};

#ifdef _DEBUG
#pragma comment(lib, "sfml-graphics-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#pragma comment(lib, "sfml-system-d.lib")
#pragma comment(lib, "sfml-network-d.lib")
#pragma comment(lib, "sfml-network-d.lib")
#else
#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-window.lib")
#pragma comment(lib, "sfml-system.lib")
#pragma comment(lib, "sfml-network.lib")
#pragma comment(lib, "sfml-network.lib")
#endif


// Global network variables
NetworkManager networkManager;
GameServer* gameServer = nullptr;
GameClient* gameClient = nullptr;
bool isMultiplayer = false;
bool isHost = false;

// Define a simple TextPanel class to display information
class TextPanel {
private:
    sf::Text text;
    sf::RectangleShape background;

public:
    // Modified constructor to handle SFML 3.0 requirements
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

// Parse command line arguments for multiplayer setup
bool parseCommandLine(int argc, char* argv[]) {
    if (argc < 2) return false;

    std::string mode = argv[1];
    if (mode == "--host") {
        isMultiplayer = true;
        isHost = true;
        unsigned short port = 5000;
        if (argc >= 3) {
            port = static_cast<unsigned short>(std::stoi(argv[2]));
        }
        return networkManager.hostGame(port);
    }
    else if (mode == "--join") {
        isMultiplayer = true;
        isHost = false;
        if (argc < 3) return false;
        std::string ip = argv[2];
        unsigned short port = 5000;
        if (argc >= 4) {
            port = static_cast<unsigned short>(std::stoi(argv[3]));
        }
        // If joining on localhost
        return networkManager.joinGame(sf::IpAddress::LocalHost, port);
    }

    return false;
}

int main(int argc, char* argv[])
{
    // Initialize SFML window
    sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), "Katie's Flight Sim");

    // Load font
    sf::Font font;
    bool fontLoaded = false;

    // Try to load from common locations based on platform
#ifdef _WIN32
// Windows font paths
    if (font.openFromFile("arial.ttf") ||
        font.openFromFile("C:/Windows/Fonts/arial.ttf") ||
        font.openFromFile("C:/Windows/Fonts/Arial.ttf")) {
        fontLoaded = true;
    }
#elif defined(__APPLE__)
// macOS font paths
    if (font.openFromFile("arial.ttf") ||
        font.openFromFile("/Library/Fonts/Arial.ttf") ||
        font.openFromFile("/System/Library/Fonts/Arial.ttf")) {
        fontLoaded = true;
    }
#elif defined(__linux__)
// Linux font paths
    if (font.openFromFile("arial.ttf") ||
        font.openFromFile("/usr/share/fonts/truetype/msttcorefonts/Arial.ttf") ||
        font.openFromFile("/usr/share/fonts/TTF/arial.ttf")) {
        fontLoaded = true;
    }
#else
// Try just the local path on other platforms
    fontLoaded = font.openFromFile("arial.ttf");
#endif

    if (!fontLoaded) {
        std::cerr << "Warning: Could not load font file. Text won't display correctly." << std::endl;
        // You might consider bundling a fallback font with your application
    }

    // Game state tracking
    GameStateA currentState = GameStateA::MENU;

    // For text input in join screen
    std::string inputAddress = "";
    std::string inputPort = "5000";
    bool focusAddress = true;  // Which input field has focus

    // Menu buttons (main menu)
    Button startButton(sf::Vector2f(640.f, 300.f), sf::Vector2f(200.f, 50.f), "Start", font,
        [&currentState]() { currentState = GameStateA::SINGLE_PLAYER; });

    Button hostButton(sf::Vector2f(640.f, 370.f), sf::Vector2f(200.f, 50.f), "Host", font,
        [&currentState]() { currentState = GameStateA::MULTIPLAYER_HOST; });

    Button joinButton(sf::Vector2f(640.f, 440.f), sf::Vector2f(200.f, 50.f), "Join", font,
        [&currentState]() { currentState = GameStateA::JOIN_MENU; });

    // Join menu buttons
    Button connectButton(sf::Vector2f(640.f, 440.f), sf::Vector2f(200.f, 50.f), "Connect", font,
        [&currentState]() { currentState = GameStateA::MULTIPLAYER_CLIENT; });

    Button backButton(sf::Vector2f(640.f, 510.f), sf::Vector2f(200.f, 50.f), "Back", font,
        [&currentState]() { currentState = GameStateA::MENU; });

    // Title text
    sf::Text titleText(font);
    titleText.setString("Katie's Flight Sim");
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(sf::Vector2f(640.f - titleText.getLocalBounds().size.x / 2, 150.f));

    // Address input display
    sf::Text addressLabel(font);
    addressLabel.setString("Server Address:");
    addressLabel.setCharacterSize(20);
    addressLabel.setFillColor(sf::Color::White);
    addressLabel.setPosition(sf::Vector2f(520.f, 300.f));

    sf::RectangleShape addressInput;
    addressInput.setSize(sf::Vector2f(300.f, 30.f));
    addressInput.setFillColor(sf::Color(50, 50, 50));
    addressInput.setOutlineColor(sf::Color::White);
    addressInput.setOutlineThickness(2.f);
    addressInput.setPosition(sf::Vector2f(520.f, 330.f));

    sf::Text addressText(font);
    addressText.setString(inputAddress);
    addressText.setCharacterSize(18);
    addressText.setFillColor(sf::Color::White);
    addressText.setPosition(sf::Vector2f(525.f, 335.f));

    // Port input display
    sf::Text portLabel(font);
    portLabel.setString("Port:");
    portLabel.setCharacterSize(20);
    portLabel.setFillColor(sf::Color::White);
    portLabel.setPosition(sf::Vector2f(520.f, 370.f));

    sf::RectangleShape portInput;
    portInput.setSize(sf::Vector2f(150.f, 30.f));
    portInput.setFillColor(sf::Color(50, 50, 50));
    portInput.setOutlineColor(sf::Color::White);
    portInput.setOutlineThickness(2.f);
    portInput.setPosition(sf::Vector2f(520.f, 400.f));

    sf::Text portText(font);
    portText.setString(inputPort);
    portText.setCharacterSize(18);
    portText.setFillColor(sf::Color::White);
    portText.setPosition(sf::Vector2f(525.f, 405.f));

    // Clock for tracking time between frames
    sf::Clock clock;

    // Main menu loop
    while (window.isOpen() && (currentState == GameStateA::MENU || currentState == GameStateA::JOIN_MENU))
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
                    {
                        if (currentState == GameStateA::JOIN_MENU)
                            currentState = GameStateA::MENU;
                        else if (currentState != GameStateA::MENU)
                            currentState = GameStateA::MENU;
                        else
                            window.close();
                    }
                    // Simple keyboard menu controls - no text or buttons needed
                    else if (keyEvent->code == sf::Keyboard::Key::Q && currentState == GameStateA::MENU)
                    {
                        // Q starts single player
                        currentState = GameStateA::SINGLE_PLAYER;
                    }
                    else if (keyEvent->code == sf::Keyboard::Key::H && currentState == GameStateA::MENU)
                    {
                        // H starts multiplayer host
                        currentState = GameStateA::MULTIPLAYER_HOST;
                    }
                    else if (keyEvent->code == sf::Keyboard::Key::J && currentState == GameStateA::MENU)
                    {
                        // J opens join menu
                        currentState = GameStateA::JOIN_MENU;
                    }
                    else if (keyEvent->code == sf::Keyboard::Key::C && currentState == GameStateA::JOIN_MENU)
                    {
                        // C connects (from join menu)
                        currentState = GameStateA::MULTIPLAYER_CLIENT;
                    }
                }
            }
        }

        // Clear the window
        window.clear(sf::Color(20, 20, 50));  // Dark blue background

        // Draw colored rectangles as visual menu indicators (no text)
        if (currentState == GameStateA::MENU)
        {
            // Draw three simple rectangles for the three menu options
            sf::RectangleShape startOption;
            startOption.setSize(sf::Vector2f(200.f, 50.f));
            startOption.setPosition(sf::Vector2f(640.f - 100.f, 300.f));
            startOption.setFillColor(sf::Color(100, 100, 100, 200));

            sf::RectangleShape hostOption;
            hostOption.setSize(sf::Vector2f(200.f, 50.f));
            hostOption.setPosition(sf::Vector2f(640.f - 100.f, 370.f));
            hostOption.setFillColor(sf::Color(100, 100, 100, 200));

            sf::RectangleShape joinOption;
            joinOption.setSize(sf::Vector2f(200.f, 50.f));
            joinOption.setPosition(sf::Vector2f(640.f - 100.f, 440.f));
            joinOption.setFillColor(sf::Color(100, 100, 100, 200));

            window.draw(startOption);
            window.draw(hostOption);
            window.draw(joinOption);
        }
        else if (currentState == GameStateA::JOIN_MENU)
        {
            // Draw two simple rectangles for the join menu
            sf::RectangleShape connectOption;
            connectOption.setSize(sf::Vector2f(200.f, 50.f));
            connectOption.setPosition(sf::Vector2f(640.f - 100.f, 340.f));
            connectOption.setFillColor(sf::Color(100, 100, 100, 200));

            sf::RectangleShape backOption;
            backOption.setSize(sf::Vector2f(200.f, 50.f));
            backOption.setPosition(sf::Vector2f(640.f - 100.f, 410.f));
            backOption.setFillColor(sf::Color(100, 100, 100, 200));

            window.draw(connectOption);
            window.draw(backOption);
        }

        // Display the window
        window.display();
    }
    // Handle state transitions
    if (currentState == GameStateA::SINGLE_PLAYER)
    {
        isMultiplayer = false;
    }
    else if (currentState == GameStateA::MULTIPLAYER_HOST)
    {
        isMultiplayer = true;
        isHost = true;
        networkManager.hostGame(5000);
    }
    else if (currentState == GameStateA::MULTIPLAYER_CLIENT)
    {
        isMultiplayer = true;
        isHost = false;

        // Parse address and port// Parse address and port
        sf::IpAddress serverAddress = sf::IpAddress::LocalHost; // Initialize with a valid value

        if (!inputAddress.empty()) {
            // In SFML 3.0, we need to use resolve
            if (auto address = sf::IpAddress::resolve(inputAddress)) {
                serverAddress = *address;
            }
            else {
                std::cerr << "Invalid IP address format" << std::endl;
                // We already initialized with LocalHost, so no need to do it again
            }
        }

        unsigned short port = static_cast<unsigned short>(std::stoi(inputPort));

        networkManager.joinGame(serverAddress, port);
    }
    else
    {
        // If we got here, the window was closed from the menu
        return 0;
    }

    // Configure network
    if (isMultiplayer) {
        networkManager.enableRobustNetworking();

        // Set a reasonable latency compensation time for client
        if (!isHost && gameClient) {
            gameClient->setLatencyCompensation(0.2f); // 200ms interpolation window
        }

        std::cout << "Network connection established. Running in "
            << (isHost ? "server" : "client") << " mode." << std::endl;
    }


    // Create a window with increased size 1280x720
    std::string windowTitle = "Katie's Space Program";
    if (isMultiplayer) {
        windowTitle += isHost ? " (Server)" : " (Client)";
    }

    // Update UI view for game
    sf::View gameView(sf::Vector2f(640.f, 360.f), sf::Vector2f(1280.f, 720.f));
    sf::View uiView(sf::Vector2f(640.f, 360.f), sf::Vector2f(1280.f, 720.f));
    float zoomLevel = 1.0f;
    float targetZoom = 1.0f;
    const float minZoom = 1.0f;      // Maximum zoom in (closest to planet)
    const float maxZoom = 1000.0f;   // Maximum zoom out (increased for larger system)
    const float zoomSpeed = 1.0f;    // Zoom speed factor

    // Create text panels for displaying information
    TextPanel rocketInfoPanel(font, 12, sf::Vector2f(10, 10), sf::Vector2f(250, 150));
    TextPanel planetInfoPanel(font, 12, sf::Vector2f(10, 170), sf::Vector2f(250, 120));
    TextPanel orbitInfoPanel(font, 12, sf::Vector2f(10, 300), sf::Vector2f(250, 100));
    TextPanel controlsPanel(font, 12, sf::Vector2f(10, 410), sf::Vector2f(250, 120));
    TextPanel multiplayerPanel(font, 12, sf::Vector2f(10, 620), sf::Vector2f(250, 90));

    // Set controls info - add multiplayer status if applicable
    std::string controlsText;
    if (isMultiplayer && !isHost) {
        // Client controls - WAD only
        controlsText =
            "CONTROLS:\n"
            "WAD: Move/Steer\n"
            "1-9: Set thrust level\n"
            "L: Transform vehicle\n"
            "Z: Zoom out\n"
            "X: Auto-zoom\n"
            "C: Focus planet 2";
    }
    else {
        // Server or single player - Arrow keys
        controlsText =
            "CONTROLS:\n"
            "Arrows: Move/Steer\n"
            "1-9: Set thrust level\n"
            "L: Transform vehicle\n"
            "Z: Zoom out\n"
            "X: Auto-zoom\n"
            "C: Focus planet 2";
    }

    if (isMultiplayer) {
        controlsText += "\n\nMULTIPLAYER MODE: ";
        controlsText += isHost ? "SERVER" : "CLIENT";
    }

    controlsPanel.setText(controlsText);

    // Gravitational constant - same as in GravitySimulator
    const float G = GameConstants::G;  // Use the constant from the header

    // Create buttons
    std::vector<Button> buttons;

    // Reference to the active vehicle manager (either from server/client or local)
    VehicleManager* activeVehicleManager = nullptr;
    std::vector<Planet*> planets;

    // Setup game objects based on multiplayer mode
    if (isMultiplayer) {
        if (isHost) {
            // Server creates and manages the game state
            gameServer = new GameServer();
            gameServer->initialize();

            // Connect NetworkManager with GameServer
            networkManager.setGameServer(gameServer);

            planets = gameServer->getPlanets();

            // Create a player for the server with ID 0
            sf::Vector2f initialPos = planets[0]->getPosition() +
                sf::Vector2f(0, -(planets[0]->getRadius() + GameConstants::ROCKET_SIZE));
            int localPlayerId = gameServer->addPlayer(0, initialPos, sf::Color::White);

            activeVehicleManager = gameServer->getPlayer(localPlayerId);

            // Setup callback to handle player input
            networkManager.onPlayerInputReceived = [](int clientId, const PlayerInput& input) {
                if (gameServer) {
                    // Use the clientId directly instead of input.playerId
                    gameServer->handlePlayerInput(clientId, input);
                }
                };
        }
        else {
            // Client receives game state from server
            gameClient = new GameClient();
            gameClient->initialize();

            // Connect NetworkManager with GameClient
            networkManager.setGameClient(gameClient);

            // Setup callback to handle game state updates
            networkManager.onGameStateReceived = [](const GameState& state) {
                if (gameClient) {
                    gameClient->processGameState(state);
                }
                };

            gameClient->setLocalPlayerId(1); // Temporary ID until server assigns one
            planets = gameClient->getPlanets();
            activeVehicleManager = gameClient->getLocalPlayer();
        }
    }
    else {
        // Single player mode - create planets directly
        // Create game objects - main planet in the center (pinned in place)
        Planet* planet = new Planet(sf::Vector2f(GameConstants::MAIN_PLANET_X, GameConstants::MAIN_PLANET_Y),
            0, GameConstants::MAIN_PLANET_MASS, sf::Color::Blue);
        // Set zero velocity to ensure it stays in place
        planet->setVelocity(sf::Vector2f(0.f, 0.f));

        // Create a second planet - position it using the calculated position
        Planet* planet2 = new Planet(sf::Vector2f(GameConstants::SECONDARY_PLANET_X, GameConstants::SECONDARY_PLANET_Y),
            0, GameConstants::SECONDARY_PLANET_MASS, sf::Color::Green);
        // Set the pre-calculated orbital velocity for a circular orbit
        planet2->setVelocity(sf::Vector2f(0.f, GameConstants::SECONDARY_PLANET_ORBITAL_VELOCITY));

        // Calculate proper orbital velocity for circular orbit using the constant distance
        // Using Kepler's laws: v = sqrt(G*M/r)
        float orbitSpeed = std::sqrt(GameConstants::G * planet->getMass() / GameConstants::PLANET_ORBIT_DISTANCE);

        // Setting velocity perpendicular to the radial direction
        planet2->setVelocity(sf::Vector2f(0.f, orbitSpeed));

        // Add planets to the list
        planets.push_back(planet);
        planets.push_back(planet2);

        // Calculate position on the first planet's edge - start at the top
        sf::Vector2f planetPos = planet->getPosition();
        float planetRadius = planet->getRadius();
        float rocketSize = GameConstants::ROCKET_SIZE; // Approximate rocket size

        // Direction vector pointing from planet center to the top (0, -1)
        sf::Vector2f direction(0, -1);
        sf::Vector2f rocketPos = planetPos + direction * (planetRadius + rocketSize);

        // Create vehicle manager with initial position
        activeVehicleManager = new VehicleManager(rocketPos, planets);
    }

    // Set up gravity simulator
    GravitySimulator gravitySimulator;
    for (auto planet : planets) {
        gravitySimulator.addPlanet(planet);
    }
    gravitySimulator.addVehicleManager(activeVehicleManager);

    // Track L key state to prevent repeated transformations
    bool lKeyPressed = false;

    // Main game loop
    while (window.isOpen())
    {
        // Calculate delta time
        float deltaTime = std::min(clock.restart().asSeconds(), 0.1f);

        // Update network state for multiplayer
        if (isMultiplayer) {
            // Update network manager to handle connections
            networkManager.update();

            // Track game time for interpolation
            static float gameTime = 0.0f;
            gameTime += deltaTime;

            if (isHost) {
                // Update server simulation
                gameServer->update(deltaTime);

                // Send updated game state to clients every 50ms (20 times per second)
                static sf::Clock stateUpdateClock;
                if (stateUpdateClock.getElapsedTime().asMilliseconds() > 50) {
                    GameState state = gameServer->getGameState();
                    networkManager.sendGameState(state);
                    stateUpdateClock.restart();
                }
            }
            else {
                // Client-side input handling with prediction

                // Get input
                PlayerInput input = gameClient->getLocalPlayerInput(deltaTime);

                // Apply input immediately for responsiveness
                gameClient->applyLocalInput(input);

                // Send to server (we'll do this more frequently than state updates)
                static sf::Clock inputUpdateClock;
                if (inputUpdateClock.getElapsedTime().asMilliseconds() > 33) { // ~30 updates per second
                    networkManager.sendPlayerInput(input);
                    inputUpdateClock.restart();
                }

                // Update client prediction and interpolation
                gameClient->update(deltaTime);
                gameClient->interpolateRemotePlayers(gameTime);
            }

            // Add network status panel (create this panel earlier with the other panels)
            std::stringstream networkStatusSS;
            networkStatusSS << "NETWORK STATUS\n";
            if (isHost) {
                networkStatusSS << "Mode: Host\n";
                networkStatusSS << "Connected Clients: " << (gameServer ? gameServer->getPlayers().size() - 1 : 0) << "\n";
            }
            else {
                networkStatusSS << "Mode: Client\n";
                networkStatusSS << "Connected: " << (networkManager.isConnected() ? "Yes" : "No") << "\n";
                networkStatusSS << "Ping: " << networkManager.getPing() << "ms\n";
                networkStatusSS << "Player ID: " << gameClient->getLocalPlayerId() << "\n";
            }
            networkStatusSS << "Packet Loss: " << networkManager.getPacketLoss();

            // Find the multiplayerPanel in your code that displays multiplayer status
            // and set its text to networkStatusSS.str()
            if (isMultiplayer) {
                multiplayerPanel.setText(networkStatusSS.str());
            }
        }

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
                    else if (keyEvent->code == sf::Keyboard::Key::L && !lKeyPressed && !isMultiplayer)
                    {
                        // Transform between rocket and car (single player only)
                        lKeyPressed = true;
                        activeVehicleManager->switchVehicle();
                    }
                    // Note: In multiplayer host mode, L key is handled in the input section
                    // In client mode, L key is captured in GameClient::getLocalPlayerInput
                }
            }

            if (event->is<sf::Event::KeyReleased>())
            {
                const auto* keyEvent = event->getIf<sf::Event::KeyReleased>();
                if (keyEvent && keyEvent->code == sf::Keyboard::Key::L)
                {
                    lKeyPressed = false;
                }
            }

            if (event->is<sf::Event::KeyReleased>())
            {
                const auto* keyEvent = event->getIf<sf::Event::KeyReleased>();
                if (keyEvent && keyEvent->code == sf::Keyboard::Key::L)
                {
                    lKeyPressed = false;
                }
            }
        }

        // In multiplayer client mode, don't handle input here as it's sent to the server
        if (!isMultiplayer) {
            // Single player - direct input handling
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1))
                activeVehicleManager->getRocket()->setThrustLevel(0.1f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2))
                activeVehicleManager->getRocket()->setThrustLevel(0.2f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3))
                activeVehicleManager->getRocket()->setThrustLevel(0.3f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num4))
                activeVehicleManager->getRocket()->setThrustLevel(0.4f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num5))
                activeVehicleManager->getRocket()->setThrustLevel(0.5f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num6))
                activeVehicleManager->getRocket()->setThrustLevel(0.6f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num7))
                activeVehicleManager->getRocket()->setThrustLevel(0.7f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num8))
                activeVehicleManager->getRocket()->setThrustLevel(0.8f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num9))
                activeVehicleManager->getRocket()->setThrustLevel(0.9f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num0))
                activeVehicleManager->getRocket()->setThrustLevel(0.0f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Equal)) // = key
                activeVehicleManager->getRocket()->setThrustLevel(1.0f);

            // Apply thrust and rotation
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
                activeVehicleManager->applyThrust(1.0f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
                activeVehicleManager->applyThrust(-0.5f); // Brake/reverse
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
                activeVehicleManager->rotate(-6.0f * deltaTime * 60.0f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
                activeVehicleManager->rotate(6.0f * deltaTime * 60.0f);
        }
        else if (isHost) {
            // Server - handle inputs for your own rocket only
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1))
                gameServer->getPlayer(0)->getRocket()->setThrustLevel(0.1f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2))
                gameServer->getPlayer(0)->getRocket()->setThrustLevel(0.2f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3))
                gameServer->getPlayer(0)->getRocket()->setThrustLevel(0.3f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num4))
                gameServer->getPlayer(0)->getRocket()->setThrustLevel(0.4f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num5))
                gameServer->getPlayer(0)->getRocket()->setThrustLevel(0.5f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num6))
                gameServer->getPlayer(0)->getRocket()->setThrustLevel(0.6f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num7))
                gameServer->getPlayer(0)->getRocket()->setThrustLevel(0.7f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num8))
                gameServer->getPlayer(0)->getRocket()->setThrustLevel(0.8f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num9))
                gameServer->getPlayer(0)->getRocket()->setThrustLevel(0.9f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num0))
                gameServer->getPlayer(0)->getRocket()->setThrustLevel(0.0f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Equal)) // = key
                gameServer->getPlayer(0)->getRocket()->setThrustLevel(1.0f);

            // Apply thrust and rotation directly to server's player with ID 0
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
                gameServer->getPlayer(0)->applyThrust(1.0f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
                gameServer->getPlayer(0)->applyThrust(-0.5f); // Brake/reverse
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
                gameServer->getPlayer(0)->rotate(-6.0f * deltaTime * 60.0f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
                gameServer->getPlayer(0)->rotate(6.0f * deltaTime * 60.0f);

            // Vehicle transformation for host player
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::L) && !lKeyPressed) {
                lKeyPressed = true;
                gameServer->getPlayer(0)->switchVehicle();
            }
        }
        // For clients, input is captured in GameClient::getLocalPlayerInput() and sent via network

        // Camera control keys
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z)) {
            // Gradually increase zoom to see more of the system
            targetZoom = std::min(maxZoom, targetZoom * 1.05f); // Increase by 5% each frame
            // Focus on active vehicle
            gameView.setCenter(activeVehicleManager->getActiveVehicle()->getPosition());
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X)) {
            // Follow the active vehicle - calculate distances manually
            float dist1 = std::sqrt(
                std::pow(activeVehicleManager->getActiveVehicle()->getPosition().x - planets[0]->getPosition().x, 2) +
                std::pow(activeVehicleManager->getActiveVehicle()->getPosition().y - planets[0]->getPosition().y, 2)
            );
            float dist2 = std::sqrt(
                std::pow(activeVehicleManager->getActiveVehicle()->getPosition().x - planets[1]->getPosition().x, 2) +
                std::pow(activeVehicleManager->getActiveVehicle()->getPosition().y - planets[1]->getPosition().y, 2)
            );
            targetZoom = minZoom + (std::min(dist1, dist2) - (planets[0]->getRadius() + GameConstants::ROCKET_SIZE)) / 100.0f;
            gameView.setCenter(activeVehicleManager->getActiveVehicle()->getPosition());
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C)) {
            // Follow planet 2
            targetZoom = 10.0f;
            gameView.setCenter(planets[1]->getPosition());
        }

        // Update simulation if not in multiplayer client mode
        if (!isMultiplayer || isHost) {
            gravitySimulator.update(deltaTime);
            for (auto planet : planets) {
                planet->update(deltaTime);
            }
            activeVehicleManager->update(deltaTime);
        }

        // Calculate distance from vehicle to closest planet for zoom
        sf::Vector2f vehiclePos = activeVehicleManager->getActiveVehicle()->getPosition();
        sf::Vector2f vehicleToPlanet1 = planets[0]->getPosition() - vehiclePos;
        sf::Vector2f vehicleToPlanet2 = planets[1]->getPosition() - vehiclePos;
        float distance1 = std::sqrt(vehicleToPlanet1.x * vehicleToPlanet1.x + vehicleToPlanet1.y * vehicleToPlanet1.y);
        float distance2 = std::sqrt(vehicleToPlanet2.x * vehicleToPlanet2.x + vehicleToPlanet2.y * vehicleToPlanet2.y);

        // Use closest planet for zoom calculation (if not manually zooming)
        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z) &&
            !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X) &&
            !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C)) {
            float closest = std::min(distance1, distance2);
            targetZoom = minZoom + (closest - (planets[0]->getRadius() + GameConstants::ROCKET_SIZE)) / 100.0f;
            targetZoom = std::max(minZoom, std::min(targetZoom, maxZoom));

            // Update view center to follow vehicle
            gameView.setCenter(vehiclePos);
        }

        // Smoothly interpolate current zoom to target zoom
        zoomLevel += (targetZoom - zoomLevel) * deltaTime * zoomSpeed;

        // Set view size based on zoom level
        gameView.setSize(sf::Vector2f(1280.f * zoomLevel, 720.f * zoomLevel));

        // Apply the view before drawing game objects
        window.setView(gameView);

        // Clear window with black background
        window.clear(sf::Color::Black);

        // Find the closest planet to the rocket
        Planet* closestPlanet = nullptr;
        float closestDistance = std::numeric_limits<float>::max();
        sf::Vector2f rocketPos = activeVehicleManager->getActiveVehicle()->getPosition();

        for (auto& planetPtr : planets) {
            sf::Vector2f direction = planetPtr->getPosition() - rocketPos;
            float dist = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (dist < closestDistance) {
                closestDistance = dist;
                closestPlanet = planetPtr;
            }
        }

        // Draw orbit path only for the closest planet
        if (closestPlanet) {
            closestPlanet->drawOrbitPath(window, planets);
        }

        // Draw trajectory only if in rocket mode
        if (activeVehicleManager->getActiveVehicleType() == VehicleType::ROCKET) {
            activeVehicleManager->getRocket()->drawTrajectory(window, planets,
                GameConstants::TRAJECTORY_TIME_STEP, GameConstants::TRAJECTORY_STEPS, false);
        }

        // Draw objects
        for (auto planet : planets) {
            planet->draw(window);
            planet->drawVelocityVector(window, 5.0f);
        }

        // Draw the active vehicle
        activeVehicleManager->drawWithConstantSize(window, zoomLevel);
        // Draw remote vehicles in multiplayer mode
        if (isMultiplayer) {
            if (isHost) {
                // Draw all player vehicles from the server
                for (const auto& pair : gameServer->getPlayers()) {
                    if (pair.second != activeVehicleManager) {
                        pair.second->drawWithConstantSize(window, zoomLevel);
                    }
                }
            }
            else {
                // Draw remote player vehicles from the client
                for (const auto& player : gameClient->getRemotePlayers()) {
                    player.second->drawWithConstantSize(window, zoomLevel);
                }
            }
        }

        // Draw velocity vector only if in rocket mode
        if (activeVehicleManager->getActiveVehicleType() == VehicleType::ROCKET) {
            activeVehicleManager->drawVelocityVector(window, 2.0f);

            // Draw gravity force vectors only in rocket mode
            activeVehicleManager->getRocket()->drawGravityForceVectors(window, planets, GameConstants::GRAVITY_VECTOR_SCALE);
        }

        // Update info panels with current data

        // 1. Vehicle information
        {
            std::stringstream ss;
            if (activeVehicleManager->getActiveVehicleType() == VehicleType::ROCKET) {
                Rocket* rocket = activeVehicleManager->getRocket();
                float speed = std::sqrt(rocket->getVelocity().x * rocket->getVelocity().x +
                    rocket->getVelocity().y * rocket->getVelocity().y);

                ss << "ROCKET INFO\n"
                    << "Mass: " << rocket->getMass() << " units\n"
                    << "Speed: " << std::fixed << std::setprecision(1) << speed << " units/s\n"
                    << "Velocity: (" << std::setprecision(1) << rocket->getVelocity().x << ", "
                    << rocket->getVelocity().y << ")\n";

                // Calculate total gravity force from all planets
                float totalForce = 0.0f;

                for (const auto& planetPtr : planets) {
                    sf::Vector2f direction = planetPtr->getPosition() - rocket->getPosition();
                    float dist = std::sqrt(direction.x * direction.x + direction.y * direction.y);

                    float forceMagnitude = G * planetPtr->getMass() * rocket->getMass() / (dist * dist);
                    totalForce += forceMagnitude;

                    // Label planets by color for clarity
                    std::string planetName = (planetPtr == planets[0]) ? "Blue Planet" : "Green Planet";
                    ss << "Force from " << planetName
                        << ": " << std::setprecision(0) << forceMagnitude << " units\n";
                }

                ss << "Total gravity force: " << std::setprecision(0) << totalForce << " units";
            }
            else {
                Car* car = activeVehicleManager->getCar();
                ss << "CAR INFO\n"
                    << "On Ground: " << (car->isOnGround() ? "Yes" : "No") << "\n"
                    << "Position: (" << std::fixed << std::setprecision(1)
                    << car->getPosition().x << ", " << car->getPosition().y << ")\n"
                    << "Orientation: " << std::setprecision(1) << car->getRotation() << " degrees\n"
                    << "Press L to transform back to rocket when on ground";
            }
            rocketInfoPanel.setText(ss.str());
        }

        // 2. Closest planet information
        {
            std::stringstream ss;
            Planet* closestPlanet = nullptr;
            float closestDistance = std::numeric_limits<float>::max();
            GameObject* activeVehicle = activeVehicleManager->getActiveVehicle();
            for (const auto& planetPtr : planets) {
                sf::Vector2f direction = planetPtr->getPosition() - activeVehicle->getPosition();
                float dist = std::sqrt(direction.x * direction.x + direction.y * direction.y);

                if (dist < closestDistance) {
                    closestDistance = dist;
                    closestPlanet = planetPtr;
                }
            }

            if (closestPlanet) {
                std::string planetName = (closestPlanet == planets[0]) ? "Blue Planet" : "Green Planet";
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

        // 3. Orbit information (only if in rocket mode)
        {
            std::stringstream ss;

            if (activeVehicleManager->getActiveVehicleType() == VehicleType::ROCKET) {
                Rocket* rocket = activeVehicleManager->getRocket();
                Planet* primaryBody = nullptr;
                float strongestGravity = 0.0f;

                // Find the primary gravitational body (usually the closest one)
                for (const auto& planetPtr : planets) {
                    sf::Vector2f direction = planetPtr->getPosition() - rocket->getPosition();
                    float dist = std::sqrt(direction.x * direction.x + direction.y * direction.y);

                    float gravityStrength = G * planetPtr->getMass() / (dist * dist);
                    if (gravityStrength > strongestGravity) {
                        strongestGravity = gravityStrength;
                        primaryBody = planetPtr;
                    }
                }

                if (primaryBody) {
                    // Calculate relative position and velocity to primary body
                    sf::Vector2f relPos = rocket->getPosition() - primaryBody->getPosition();
                    sf::Vector2f relVel = rocket->getVelocity() - primaryBody->getVelocity();

                    // Calculate orbit parameters
                    float apoapsis = calculateApoapsis(relPos, relVel, primaryBody->getMass(), G);
                    float periapsis = calculatePeriapsis(relPos, relVel, primaryBody->getMass(), G);

                    // Current distance
                    float distance = std::sqrt(relPos.x * relPos.x + relPos.y * relPos.y);

                    // Check if orbit is valid (not hyperbolic)
                    if (periapsis > 0 && apoapsis > 0) {
                        ss << "ORBIT INFO\n"
                            << "Primary: " << (primaryBody == planets[0] ? "Blue Planet" : "Green Planet") << "\n"
                            << "Periapsis: " << std::fixed << std::setprecision(0) << periapsis << " units\n"
                            << "Apoapsis: " << std::setprecision(0) << apoapsis << " units\n"
                            << "Current dist: " << std::setprecision(0) << distance << " units";
                    }
                    else {
                        ss << "ORBIT INFO\n"
                            << "Primary: " << (primaryBody == planets[0] ? "Blue Planet" : "Green Planet") << "\n"
                            << "Orbit: Escape trajectory\n"
                            << "Current dist: " << std::fixed << std::setprecision(0) << distance << " units";
                    }
                }
            }
            else {
                ss << "ORBIT INFO\n"
                    << "Not available in car mode\n"
                    << "Transform to rocket for orbital data";
            }

            orbitInfoPanel.setText(ss.str());
        }

        // 4. Thrust metrics panel content (prepare the content but don't draw yet)
        TextPanel thrustMetricsPanel(font, 12, sf::Vector2f(10, 530), sf::Vector2f(250, 80));
        if (activeVehicleManager->getActiveVehicleType() == VehicleType::ROCKET) {
            Rocket* rocket = activeVehicleManager->getRocket();
            sf::Vector2f rocketPos = rocket->getPosition();

            // Calculate the closest planet for gravity reference
            Planet* closestPlanet = nullptr;
            float closestDistance = std::numeric_limits<float>::max();

            for (const auto& planetPtr : planets) {
                float dist = std::sqrt(std::pow(rocketPos.x - planetPtr->getPosition().x, 2) + std::pow(rocketPos.y - planetPtr->getPosition().y, 2));
                if (dist < closestDistance) {
                    closestDistance = dist;
                    closestPlanet = planetPtr;
                }
            }

            if (closestPlanet) {
                // Calculate gravity force and direction
                sf::Vector2f towardsPlanet = closestPlanet->getPosition() - rocketPos;
                float dist = std::sqrt(towardsPlanet.x * towardsPlanet.x + towardsPlanet.y * towardsPlanet.y);
                sf::Vector2f gravityDir = normalize(towardsPlanet);
                // Calculate weight (gravity force) at current position
                float weight = G * rocket->getMass() * closestPlanet->getMass() / (dist * dist);

                // Calculate current thrust force based on thrust level
                float maxThrust = 0.0f;
                for (const auto& part : rocket->getParts()) {
                    if (auto* engine = dynamic_cast<Engine*>(part.get())) {
                        maxThrust += engine->getThrust();
                    }
                }
                float currentThrust = maxThrust * rocket->getThrustLevel();

                // Calculate expected acceleration (thrust/mass - gravity)
                float thrustToWeightRatio = currentThrust / (weight > 0 ? weight : 1.0f);

                // Calculate expected acceleration along the thrust direction
                float radians = rocket->getRotation() * 3.14159f / 180.0f;
                sf::Vector2f thrustDir(std::sin(radians), -std::cos(radians));

                // Project gravity onto thrust direction (negative if opposing thrust)
                float projectedGravity = gravityDir.x * thrustDir.x + gravityDir.y * thrustDir.y;
                float gravityComponent = weight * projectedGravity;

                // Net acceleration along thrust direction
                float netAccel = (currentThrust - gravityComponent) / rocket->getMass();

                // Set the text content
                std::stringstream ss;
                ss << "THRUST METRICS\n"
                    << "Thrust Level: " << std::fixed << std::setprecision(2) << rocket->getThrustLevel() * 100.0f << "%\n"
                    << "Thrust-to-Weight Ratio: " << std::setprecision(2) << thrustToWeightRatio << "\n"
                    << "Expected Acceleration: " << std::setprecision(2) << netAccel << " units/s²\n"
                    << "Escape Velocity: " << std::setprecision(0)
                    << std::sqrt(2.0f * G * closestPlanet->getMass() / dist) << " units/s";

                thrustMetricsPanel.setText(ss.str());
            }
            else {
                thrustMetricsPanel.setText("THRUST METRICS\nNo planet in range");
            }
        }
        else {
            thrustMetricsPanel.setText("THRUST METRICS\nNot available in car mode");
        }

        // Add multiplayer info panel
        TextPanel multiplayerPanel(font, 12, sf::Vector2f(10, 620), sf::Vector2f(250, 90));
        if (isMultiplayer) {
            std::stringstream ss;
            ss << "MULTIPLAYER STATUS\n";
            ss << "Mode: " << (isHost ? "Host" : "Client") << "\n";

            if (isHost && gameServer) {
                ss << "Connected clients: " << (gameServer->getPlayers().size() - 1) << "\n";
            }
            else {
                ss << "Connected to server: " << (networkManager.isConnected() ? "Yes" : "No") << "\n";
                ss << "Local player ID: " << gameClient->getLocalPlayerId() << "\n";
                ss << "Remote players: " << gameClient->getRemotePlayers().size() << "\n";
            }
            multiplayerPanel.setText(ss.str());
        }
        else {
            multiplayerPanel.setText("SINGLE PLAYER MODE\nPress ESC to exit");
        }

        // Now switch to UI view for drawing all panels
        window.setView(uiView);

        // Draw all panels
        rocketInfoPanel.draw(window);
        planetInfoPanel.draw(window);
        orbitInfoPanel.draw(window);
        controlsPanel.draw(window);
        thrustMetricsPanel.draw(window);
        if (isMultiplayer) {
            multiplayerPanel.draw(window);
        }

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

   // Cleanup
   if (!isMultiplayer) {
       // In single player mode, clean up our own objects
       delete activeVehicleManager;
       for (auto planet : planets) {
           delete planet;
       }
   }
   else {
       // In multiplayer mode, clean up server/client
       delete gameServer;
       delete gameClient;
   }

   return 0;
}