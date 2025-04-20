#pragma once
#include "GameObject.h"

class Planet : public GameObject {
private:
    sf::CircleShape shape;
    float mass;
    float radius;

public:
    Planet(sf::Vector2f pos, float radius, float mass, sf::Color color = sf::Color::Blue);

    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) override;

    float getMass() const;
    float getRadius() const;
};