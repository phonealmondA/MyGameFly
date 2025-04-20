#pragma once
#include "D:/MyGameFly/SFML-3.0.0-windows-vc17-64-bit/SFML-3.0.0/include/SFML/Graphics.hpp"

class GameObject {
protected:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color color;

public:
    GameObject(sf::Vector2f pos, sf::Vector2f vel, sf::Color col);
    virtual ~GameObject() = default;

    virtual void update(float deltaTime) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;

    sf::Vector2f getPosition() const;
    sf::Vector2f getVelocity() const;
    void setVelocity(sf::Vector2f vel);
};