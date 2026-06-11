#pragma once

#include <vector>

#include <memory>

#include <SFML/Graphics.hpp>

#include "triangle.h"

#include "transform.h"

#include "camera.h"

#include "light.h"

class Mesh {

public:

    std::vector<std::unique_ptr<Triangle>> triangles;

    sf::Color color;

    Transform transform;

    bool shadeSmooth = false;

    Mesh(std::vector<std::unique_ptr<Triangle>> tris, sf::Color c, Transform t = Transform()) : triangles(std::move(tris)), color(c), transform(t) {}

    void rasterize(sf::Image& colorBuffer, std::vector<float>& depthBuffer, const Camera& cam, int width, int height, const Light& light, const Light::LightCamera* lightCam = nullptr, const std::vector<float>* shadowMap = nullptr, int shadowMapSize = 0) const;

    void rasterizeShadow(std::vector<float>& shadowDepthBuffer, const Light::LightCamera& lightCam, int shadowMapSize) const;

    void drawWireframe(sf::RenderWindow& window, const Camera& cam, int width, int height) const;

};
