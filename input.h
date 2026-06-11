#pragma once

#include <SFML/Graphics.hpp>
#include "debug.h"
#include "camera.h"

void handleInput(DebugState& debugState, Camera& cam, float& yaw, float& pitch, float radius, float dt, sf::RenderWindow& window, bool& tabLast, bool& num1Last, bool& num2Last, bool& num3Last, bool& num4Last);
