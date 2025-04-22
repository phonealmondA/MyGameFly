// PlayerInput.cpp
#include "PlayerInput.h"

// Implement serialization for PlayerInput
sf::Packet& operator<<(sf::Packet& packet, const PlayerInput& input) {
    return packet << input.playerId
        << input.thrustForward << input.thrustBackward
        << input.rotateLeft << input.rotateRight
        << input.switchVehicle << input.thrustLevel
        << input.deltaTime;
}

sf::Packet& operator>>(sf::Packet& packet, PlayerInput& input) {
    return packet >> input.playerId
        >> input.thrustForward >> input.thrustBackward
        >> input.rotateLeft >> input.rotateRight
        >> input.switchVehicle >> input.thrustLevel
        >> input.deltaTime;
}