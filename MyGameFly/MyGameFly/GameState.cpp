// GameState.cpp
#include "GameState.h"

// Implement serialization for sf::Vector2f
sf::Packet& operator<<(sf::Packet& packet, const sf::Vector2f& vector) {
    return packet << vector.x << vector.y;
}

sf::Packet& operator>>(sf::Packet& packet, sf::Vector2f& vector) {
    return packet >> vector.x >> vector.y;
}

// Implement serialization for sf::Color
sf::Packet& operator<<(sf::Packet& packet, const sf::Color& color) {
    return packet << color.r << color.g << color.b << color.a;
}

sf::Packet& operator>>(sf::Packet& packet, sf::Color& color) {
    return packet >> color.r >> color.g >> color.b >> color.a;
}

// Implement RocketState serialization
sf::Packet& operator<<(sf::Packet& packet, const RocketState& state) {
    return packet << state.playerId << state.position << state.velocity
        << state.rotation << state.angularVelocity << state.thrustLevel
        << state.mass << state.color;
}

sf::Packet& operator>>(sf::Packet& packet, RocketState& state) {
    return packet >> state.playerId >> state.position >> state.velocity
        >> state.rotation >> state.angularVelocity >> state.thrustLevel
        >> state.mass >> state.color;
}

// Implement PlanetState serialization
sf::Packet& operator<<(sf::Packet& packet, const PlanetState& state) {
    return packet << state.planetId << state.position << state.velocity
        << state.mass << state.radius << state.color;
}

sf::Packet& operator>>(sf::Packet& packet, PlanetState& state) {
    return packet >> state.planetId >> state.position >> state.velocity
        >> state.mass >> state.radius >> state.color;
}

// Implement GameState serialization
sf::Packet& operator<<(sf::Packet& packet, const GameState& state) {
    packet << state.sequenceNumber << state.timestamp;

    // Serialize rockets
    packet << static_cast<sf::Uint32>(state.rockets.size());
    for (const auto& rocket : state.rockets) {
        packet << rocket;
    }

    // Serialize planets
    packet << static_cast<sf::Uint32>(state.planets.size());
    for (const auto& planet : state.planets) {
        packet << planet;
    }

    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, GameState& state) {
    packet >> state.sequenceNumber >> state.timestamp;

    // Deserialize rockets
    sf::Uint32 rocketCount;
    packet >> rocketCount;
    state.rockets.resize(rocketCount);
    for (sf::Uint32 i = 0; i < rocketCount; ++i) {
        packet >> state.rockets[i];
    }

    // Deserialize planets
    sf::Uint32 planetCount;
    packet >> planetCount;
    state.planets.resize(planetCount);
    for (sf::Uint32 i = 0; i < planetCount; ++i) {
        packet >> state.planets[i];
    }

    return packet;
}