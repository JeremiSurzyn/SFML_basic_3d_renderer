#pragma once

#include "vmath.h"
#include "camera.h"
#include "light.h"
#include <SFML/Graphics.hpp>
#include <vector>



class Triangle
{
public:
    Triangle(const Vector3f& v1, const Vector3f& v2, const Vector3f& v3, const sf::Color& color);

    void setColor(const sf::Color& color);
    sf::Color getColor() const;
    void setVertex(int index, const Vector3f& vertex);
    Vector3f getVertex(int index) const;
    Vector3f getNormal() const {
        Vector3f edge1 = vertices[1] - vertices[0];
        Vector3f edge2 = vertices[2] - vertices[0];
        return vm::cross(edge1, edge2).normalized();
    }

    void transform(const Matrix4f& mat);

private:
    Vector3f vertices[3];
    sf::Color color;
public:
    static void rasterise(sf::Image& colorBuffer, std::vector<float>& depthBuffer, const Triangle& t, const Camera& cam, int width, int height, sf::Color color, const std::vector<float>* shadowMap = nullptr, int shadowMapSize = 0, const Light::LightCamera* lightCam = nullptr);

    static void rasterise(sf::Image& colorBuffer, std::vector<float>& depthBuffer, const Triangle& t, const Camera& cam, int width, int height, const sf::Color& c0, const sf::Color& c1, const sf::Color& c2, const std::vector<float>* shadowMap = nullptr, int shadowMapSize = 0, const Light::LightCamera* lightCam = nullptr);


    static void rasteriseShadow(std::vector<float>& shadowDepthBuffer, const Triangle& t, const Light::LightCamera& lightCam, int shadowMapSize);

    static void drawWireframe(sf::RenderWindow& window, const Triangle& t, const Camera& cam, int width, int height, sf::Color c);
};
