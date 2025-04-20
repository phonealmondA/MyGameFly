#include "Rocket.h"
#include "VectorHelper.h"
#include <cmath>

Rocket::Rocket(sf::Vector2f pos, sf::Vector2f vel, sf::Color col)
    : GameObject(pos, vel, col), rotation(0), angularVelocity(0), thrustLevel(0.0f)
{
    // Create rocket body (a simple triangle)
    body.setPointCount(3);
    body.setPoint(0, { 0, -20 });
    body.setPoint(1, { -10, 20 });
    body.setPoint(2, { 10, 20 });
    body.setFillColor(color);
    body.setOrigin({ 0, 0 });
    body.setPosition(position);

    // Add default engine
    addPart(std::make_unique<Engine>(sf::Vector2f(0, 15), 100.0f));
}

void Rocket::addPart(std::unique_ptr<RocketPart> part)
{
    parts.push_back(std::move(part));
}
void Rocket::applyThrust(float amount)
{
    // Calculate thrust direction based on rocket rotation
    float radians = rotation * 3.14159f / 180.0f;

    // In SFML, 0 degrees points up, 90 degrees points right
    // So we need to use -sin for x and -cos for y to get the direction
    sf::Vector2f thrustDir(std::sin(radians), -std::cos(radians));

    velocity += thrustDir * amount * thrustLevel;
}
void Rocket::rotate(float amount)
{
    angularVelocity += amount;
}

void Rocket::setThrustLevel(float level)
{
    // Clamp level between 0.0 and 1.0
    thrustLevel = std::max(0.0f, std::min(1.0f, level));
}

bool Rocket::checkCollision(const Planet& planet)
{
    float dist = distance(position, planet.getPosition());
    // Simple collision check based on distance
    return dist < planet.getRadius() + 15.0f; // 15 = approximate rocket size
}

bool Rocket::isColliding(const Planet& planet)
{
    return checkCollision(planet);
}

void Rocket::update(float deltaTime)
{
    // Update position based on velocity
    position += velocity * deltaTime;

    // Update rotation based on angular velocity
    rotation += angularVelocity * deltaTime;

    // Apply some damping to angular velocity
    angularVelocity *= 0.98f;

    // Update body position and rotation
    body.setPosition(position);
    body.setRotation(sf::degrees(rotation));
}

void Rocket::draw(sf::RenderWindow& window)
{
    // Draw rocket body
    window.draw(body);

    // Draw all rocket parts
    for (const auto& part : parts) {
        part->draw(window, position, rotation);
    }
}

void Rocket::drawVelocityVector(sf::RenderWindow& window, float scale)
{
    sf::VertexArray line(sf::PrimitiveType::LineStrip);

    sf::Vertex startVertex;
    startVertex.position = position;
    startVertex.color = sf::Color::Yellow;
    line.append(startVertex);

    sf::Vertex endVertex;
    endVertex.position = position + velocity * scale;
    endVertex.color = sf::Color::Red;
    line.append(endVertex);

    window.draw(line);
}

void Rocket::drawTrajectory(sf::RenderWindow& window, const std::vector<Planet*>& planets,
    float timeStep, int steps) {
    // Create a vertex array for the trajectory line
    sf::VertexArray trajectory(sf::PrimitiveType::LineStrip);

    // Start with current position and velocity
    sf::Vector2f simPosition = position;
    sf::Vector2f simVelocity = velocity;

    // Add the starting point
    sf::Vertex startPoint;
    startPoint.position = simPosition;
    startPoint.color = sf::Color::Blue; // Blue at the beginning
    trajectory.append(startPoint);

    // Simulate future positions
    for (int i = 0; i < steps; i++) {
        // Calculate gravitational forces from all planets
        sf::Vector2f totalAcceleration(0, 0);

        for (const auto& planet : planets) {
            sf::Vector2f direction = planet->getPosition() - simPosition;
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            // Skip if inside the planet
            if (distance <= planet->getRadius() + 10.0f) {
                // Stop the trajectory if we hit a planet
                break;
            }

            // Apply constant gravity like in GravitySimulator
            const float constantForce = 100.0f; // Same as in GravitySimulator
            sf::Vector2f acceleration = normalize(direction) * constantForce;

            totalAcceleration += acceleration;
        }

        // Update simulated velocity and position
        simVelocity += totalAcceleration * timeStep;
        simPosition += simVelocity * timeStep;

        // Calculate color gradient from blue to pink
        float ratio = static_cast<float>(i) / steps;
        sf::Color pointColor(
            51 + 204 * ratio,  // R: 51 (blue) to 255 (pink)
            51 + 0 * ratio,    // G: 51 (blue) to 51 (pink)
            255 - 155 * ratio  // B: 255 (blue) to 100 (pink)
        );

        // Add point to trajectory
        sf::Vertex point;
        point.position = simPosition;
        point.color = pointColor;
        trajectory.append(point);
    }

    // Draw the trajectory
    window.draw(trajectory);
}