#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "camera.h"
#include "mesh.h"

struct DebugState {
    bool showOverlay = false;
    int viewMode = 0;
    bool showImportedObjects = true;
    float fps = 0.0f;
};

void updateDebugInput(DebugState& state, bool tabPressed, bool num1Pressed, bool num2Pressed, bool num3Pressed, bool num4Pressed,
                      bool& tabLast, bool& num1Last, bool& num2Last, bool& num3Last, bool& num4Last);

void drawDebugOverlay(sf::RenderWindow& window, const DebugState& state, sf::Font& font);
