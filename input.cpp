#include "input.h"
#include <cmath>

void handleInput(DebugState& debugState, Camera& cam, float& yaw, float& pitch, float radius, float dt, sf::RenderWindow& window, bool& tabLast, bool& num1Last, bool& num2Last, bool& num3Last, bool& num4Last)
{
    bool tabPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Tab);
    bool num1Pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1);
    bool num2Pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2);
    bool num3Pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3);
    bool num4Pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num4);

    updateDebugInput(debugState, tabPressed, num1Pressed, num2Pressed, num3Pressed, num4Pressed, tabLast, num1Last, num2Last, num3Last, num4Last);

    const float rotSpeed = 1.5f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) yaw -= rotSpeed * dt;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) yaw += rotSpeed * dt;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) pitch += rotSpeed * dt;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) pitch -= rotSpeed * dt;

    const float halfPi = 1.57079632679f - 0.01f;
    if (pitch > halfPi)  pitch = halfPi;
    if (pitch < -halfPi) pitch = -halfPi;

    cam.position.x = radius * std::cos(pitch) * std::cos(yaw);
    cam.position.y = radius * std::sin(pitch);
    cam.position.z = radius * std::cos(pitch) * std::sin(yaw);
}
