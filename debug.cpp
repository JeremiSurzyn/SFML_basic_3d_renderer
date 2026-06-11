#include "debug.h"

void updateDebugInput(DebugState& state, bool tabPressed, bool num1Pressed, bool num2Pressed, bool num3Pressed, bool num4Pressed,
                      bool& tabLast, bool& num1Last, bool& num2Last, bool& num3Last, bool& num4Last) {
    state.showOverlay = tabPressed;

    if (tabPressed) {
        if (num1Pressed && !num1Last) {
            state.viewMode = 1;
        }
        if (num2Pressed && !num2Last) {
            state.viewMode = 0;
        }
        if (num3Pressed && !num3Last) {
            state.viewMode = 2;
        }
        if (num4Pressed && !num4Last) {
            state.showImportedObjects = !state.showImportedObjects;
        }
    }

    num1Last = num1Pressed;
    num2Last = num2Pressed;
    num3Last = num3Pressed;
    num4Last = num4Pressed;
    tabLast = tabPressed;
}

void drawDebugOverlay(sf::RenderWindow& window, const DebugState& state, sf::Font& font) {
    if (!state.showOverlay) return;

    sf::Text text(font);
    text.setCharacterSize(20);
    text.setFillColor(sf::Color::White);
    text.setPosition(sf::Vector2f(10, 10));

    std::string str = "FPS: " + std::to_string((int)state.fps) + "\n";
    str += "Debug Keys:\n";
    str += "Hold Tab: Show overlay\n";
    str += "Tab + 1: Depth view\n";
    str += "Tab + 2: Rasterization\n";
    str += "Tab + 3: Shadow map\n";
    str += "Tab + 4: Toggle imported/primitives\n";
    str += "Mode: ";
    str += state.showImportedObjects ? "Imported\n" : "Primitives\n";

    text.setString(str);
    window.draw(text);
}
