#include "Planet.h"

Planet::Planet(sf::Vector2f pos, float radius, float mass, sf::Color color)
    : GameObject(pos, { 0, 0 }, color), mass(mass), radius(radius)
{
    shape.setRadius(radius);
    shape.setFillColor(color);
    shape.setOrigin({ radius, radius });
    shape.setPosition(position);
}

void Planet::update(float deltaTime)
{
    position += velocity * deltaTime;
    shape.setPosition(position);
}

void Planet::draw(sf::RenderWindow& window)
{
    window.draw(shape);
}

float Planet::getMass() const
{
    return mass;
}

float Planet::getRadius() const
{
    return radius;
}