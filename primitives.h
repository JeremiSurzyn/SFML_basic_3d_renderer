#pragma once

#include "mesh.h"
#include <SFML/Graphics.hpp>

Mesh createCube(float size, sf::Color color, const Transform& transform = Transform());
Mesh createIcosahedron(float radius, sf::Color color, const Transform& transform = Transform());
Mesh createIcosphere(float radius, int subdivisions, sf::Color color, const Transform& transform = Transform());
