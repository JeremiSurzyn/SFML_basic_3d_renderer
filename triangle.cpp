#include "triangle.h"
#include "light.h"
#include <algorithm>
#include <vector>
#include <cmath>

Triangle::Triangle(const Vector3f& v1, const Vector3f& v2, const Vector3f& v3, const sf::Color& color)
{
    vertices[0] = v1;
    vertices[1] = v2;
    vertices[2] = v3;
    this->color = color;
}

void Triangle::setColor(const sf::Color& c)
{
    color = c;
}

sf::Color Triangle::getColor() const
{
    return color;
}

void Triangle::setVertex(int index, const Vector3f& vertex)
{
    if (index >= 0 && index < 3)
        vertices[index] = vertex;
}

Vector3f Triangle::getVertex(int index) const
{
    if (index >= 0 && index < 3)
        return vertices[index];
    return {0,0,0};
}

void Triangle::transform(const Matrix4f& mat) {
    vertices[0] = mat * vertices[0];
    vertices[1] = mat * vertices[1];
    vertices[2] = mat * vertices[2];
}

struct Barycentric {
    float u, v, w;
};

Barycentric getBarycentric(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b, sf::Vector2f c) {
    sf::Vector2f v0 = b - a;
    sf::Vector2f v1 = c - a;
    sf::Vector2f v2 = p - a;

    float dot00 = v0.x * v0.x + v0.y * v0.y;
    float dot01 = v0.x * v1.x + v0.y * v1.y;
    float dot02 = v0.x * v2.x + v0.y * v2.y;
    float dot11 = v1.x * v1.x + v1.y * v1.y;
    float dot12 = v1.x * v2.x + v1.y * v2.y;

    float denom = dot00 * dot11 - dot01 * dot01;
    if (std::abs(denom) < 1e-6f) return {0, 0, 0};
    float invDenom = 1.0f / denom;

    float v = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float w = (dot00 * dot12 - dot01 * dot02) * invDenom;
    float u = 1.0f - v - w;

    return {u, v, w};
}


bool isInShadow(float pixelDepth, const std::vector<float>& shadowMap, int shadowMapSize, const Light::LightCamera& lightCam, const Vector3f& worldPos) {
    if (shadowMap.empty()) return false;


    Vector3f lightProj = lightCam.project(worldPos, shadowMapSize);

    if (lightProj.x < 0 || lightProj.y < 0 || lightProj.x >= shadowMapSize || lightProj.y >= shadowMapSize) {

        return false;
    }

    int cx = static_cast<int>(std::floor(lightProj.x));
    int cy = static_cast<int>(std::floor(lightProj.y));
    const float shadowBias = 0.05f*lightProj.z;
    int occluded = 0;
    int samples = 0;

    for (int oy = -1; oy <= 1; ++oy) {
        for (int ox = -1; ox <= 1; ++ox) {
            int sx = cx + ox;
            int sy = cy + oy;
            if (sx < 0 || sx >= shadowMapSize || sy < 0 || sy >= shadowMapSize) continue;
            float shadowDepth = shadowMap[sy * shadowMapSize + sx];
            if (lightProj.z > shadowDepth + shadowBias) {
                ++occluded;
            }
            ++samples;
        }
    }

    if (samples == 0) return false;
    return occluded * 2 > samples;
}

sf::Vector2f projectToScreen(const Vector3f& v, const Camera& cam, int width, int height) {
    Vector3f p = cam.project(v, width, height);
    return sf::Vector2f{p.x, p.y};
}


void Triangle::rasteriseShadow(std::vector<float>& shadowDepthBuffer, const Triangle& t, const Light::LightCamera& lightCam, int shadowMapSize) {

    Vector3f proj0 = lightCam.project(t.getVertex(0), shadowMapSize);
    Vector3f proj1 = lightCam.project(t.getVertex(1), shadowMapSize);
    Vector3f proj2 = lightCam.project(t.getVertex(2), shadowMapSize);


    if (proj0.z < 0 || proj1.z < 0 || proj2.z < 0) return;

    sf::Vector2f s0 = {proj0.x, proj0.y};
    sf::Vector2f s1 = {proj1.x, proj1.y};
    sf::Vector2f s2 = {proj2.x, proj2.y};

    float z0 = proj0.z;
    float z1 = proj1.z;
    float z2 = proj2.z;


    float minX = std::min({s0.x, s1.x, s2.x});
    float maxX = std::max({s0.x, s1.x, s2.x});
    float minY = std::min({s0.y, s1.y, s2.y});
    float maxY = std::max({s0.y, s1.y, s2.y});


    int startX = std::max(0, (int)std::floor(minX));
    int endX = std::min(shadowMapSize - 1, (int)std::ceil(maxX));
    int startY = std::max(0, (int)std::floor(minY));
    int endY = std::min(shadowMapSize - 1, (int)std::ceil(maxY));

    sf::Vector2f a = s0;
    sf::Vector2f b = s1;
    sf::Vector2f c = s2;


    sf::Vector2f ab = b - a;
    sf::Vector2f bc = c - b;
    sf::Vector2f ca = a - c;


    auto edge = [](const sf::Vector2f& a, const sf::Vector2f& b, const sf::Vector2f& p) {
        return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
    };

    float area = edge(a, b, c);


    if (std::abs(area) < 1e-6f) return;

    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {

            sf::Vector2f p(x + 0.5f, y + 0.5f);

            float w0 = edge(b, c, p);
            float w1 = edge(c, a, p);
            float w2 = edge(a, b, p);


            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {


                w0 /= area;
                w1 /= area;
                w2 /= area;


                float depth = w0 * z0 + w1 * z1 + w2 * z2;

                int index = y * shadowMapSize + x;

                if (depth < shadowDepthBuffer[index]) {
                    shadowDepthBuffer[index] = depth;
                }
            }
        }
    }
}

void Triangle::drawWireframe(sf::RenderWindow& window, const Triangle& t, const Camera& cam, int width, int height, sf::Color c) {
    sf::Vector2f v0 = projectToScreen(t.getVertex(0), cam, width, height);
    sf::Vector2f v1 = projectToScreen(t.getVertex(1), cam, width, height);
    sf::Vector2f v2 = projectToScreen(t.getVertex(2), cam, width, height);

    sf::VertexArray va(sf::PrimitiveType::Lines, 6);
    va[0].position = v0; va[0].color = c;
    va[1].position = v1; va[1].color = c;
    va[2].position = v1; va[2].color = c;
    va[3].position = v2; va[3].color = c;
    va[4].position = v2; va[4].color = c;
    va[5].position = v0; va[5].color = c;
    window.draw(va);
}

void Triangle::rasterise(sf::Image& colorBuffer, std::vector<float>& depthBuffer, const Triangle& t, const Camera& cam, int width, int height, sf::Color color, const std::vector<float>* shadowMap, int shadowMapSize, const Light::LightCamera* lightCam) {

    Vector3f proj0 = cam.project(t.getVertex(0), width, height);
    Vector3f proj1 = cam.project(t.getVertex(1), width, height);
    Vector3f proj2 = cam.project(t.getVertex(2), width, height);


    if (proj0.z < 0 || proj1.z < 0 || proj2.z < 0) return;

    sf::Vector2f s0 = {proj0.x, proj0.y};
    sf::Vector2f s1 = {proj1.x, proj1.y};
    sf::Vector2f s2 = {proj2.x, proj2.y};

    float z0 = proj0.z;
    float z1 = proj1.z;
    float z2 = proj2.z;


    float minX = std::min({s0.x, s1.x, s2.x});
    float maxX = std::max({s0.x, s1.x, s2.x});
    float minY = std::min({s0.y, s1.y, s2.y});
    float maxY = std::max({s0.y, s1.y, s2.y});


    int startX = std::max(0, (int)std::floor(minX));
    int endX = std::min(width - 1, (int)std::ceil(maxX));
    int startY = std::max(0, (int)std::floor(minY));
    int endY = std::min(height - 1, (int)std::ceil(maxY));

    float invZ0 = 1.0f / z0;
    float invZ1 = 1.0f / z1;
    float invZ2 = 1.0f / z2;

    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            sf::Vector2f p(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);
            Barycentric bary = getBarycentric(p, s0, s1, s2);
            if (bary.u >= 0.0f && bary.v >= 0.0f && bary.w >= 0.0f) {
                int index = y * width + x;
                if (cam.isPerspective) {
                    float invZ = bary.u * invZ0 + bary.v * invZ1 + bary.w * invZ2;
                    float depth = 1.0f / invZ;
                    if (depth < depthBuffer[index]) {
                        float w0 = bary.u * invZ0;
                        float w1 = bary.v * invZ1;
                        float w2 = bary.w * invZ2;
                        float weight = w0 + w1 + w2;
                        Vector3f worldPos = (t.getVertex(0) * w0 + t.getVertex(1) * w1 + t.getVertex(2) * w2) * (1.0f / weight);
                        sf::Color finalColor = color;
                        if (shadowMap && shadowMapSize > 0 && lightCam) {
                            if (isInShadow(depth, *shadowMap, shadowMapSize, *lightCam, worldPos)) {
                                finalColor = sf::Color(
                                    static_cast<uint8_t>(color.r * 0.3f),
                                    static_cast<uint8_t>(color.g * 0.3f),
                                    static_cast<uint8_t>(color.b * 0.3f)
                                );
                            }
                        }
                        colorBuffer.setPixel(sf::Vector2u(x, y), finalColor);
                        depthBuffer[index] = depth;
                    }
                } else {
                    float depth = bary.u * z0 + bary.v * z1 + bary.w * z2;
                    if (depth < depthBuffer[index]) {
                        Vector3f worldPos = t.getVertex(0) * bary.u + t.getVertex(1) * bary.v + t.getVertex(2) * bary.w;
                        sf::Color finalColor = color;
                        if (shadowMap && shadowMapSize > 0 && lightCam) {
                            if (isInShadow(depth, *shadowMap, shadowMapSize, *lightCam, worldPos)) {
                                finalColor = sf::Color(
                                    static_cast<uint8_t>(color.r * 0.3f),
                                    static_cast<uint8_t>(color.g * 0.3f),
                                    static_cast<uint8_t>(color.b * 0.3f)
                                );
                            }
                        }
                        colorBuffer.setPixel(sf::Vector2u(x, y), finalColor);
                        depthBuffer[index] = depth;
                    }
                }
            }
        }
    }
}

void Triangle::rasterise(sf::Image& colorBuffer, std::vector<float>& depthBuffer, const Triangle& t, const Camera& cam, int width, int height, const sf::Color& c0, const sf::Color& c1, const sf::Color& c2, const std::vector<float>* shadowMap, int shadowMapSize, const Light::LightCamera* lightCam) {

    Vector3f proj0 = cam.project(t.getVertex(0), width, height);
    Vector3f proj1 = cam.project(t.getVertex(1), width, height);
    Vector3f proj2 = cam.project(t.getVertex(2), width, height);


    if (proj0.z < 0 || proj1.z < 0 || proj2.z < 0) return;

    sf::Vector2f s0 = {proj0.x, proj0.y};
    sf::Vector2f s1 = {proj1.x, proj1.y};
    sf::Vector2f s2 = {proj2.x, proj2.y};

    float z0 = proj0.z;
    float z1 = proj1.z;
    float z2 = proj2.z;


    float minX = std::min({s0.x, s1.x, s2.x});
    float maxX = std::max({s0.x, s1.x, s2.x});
    float minY = std::min({s0.y, s1.y, s2.y});
    float maxY = std::max({s0.y, s1.y, s2.y});


    int startX = std::max(0, (int)std::floor(minX));
    int endX = std::min(width - 1, (int)std::ceil(maxX));
    int startY = std::max(0, (int)std::floor(minY));
    int endY = std::min(height - 1, (int)std::ceil(maxY));

    float invZ0 = 1.0f / z0;
    float invZ1 = 1.0f / z1;
    float invZ2 = 1.0f / z2;

    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            sf::Vector2f p(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);
            Barycentric bary = getBarycentric(p, s0, s1, s2);
            if (bary.u >= 0.0f && bary.v >= 0.0f && bary.w >= 0.0f) {
                int index = y * width + x;
                if (cam.isPerspective) {
                    float invZ = bary.u * invZ0 + bary.v * invZ1 + bary.w * invZ2;
                    float depth = 1.0f / invZ;
                    if (depth < depthBuffer[index]) {
                        float r = (bary.u * (c0.r * invZ0) + bary.v * (c1.r * invZ1) + bary.w * (c2.r * invZ2)) / invZ;
                        float g = (bary.u * (c0.g * invZ0) + bary.v * (c1.g * invZ1) + bary.w * (c2.g * invZ2)) / invZ;
                        float b = (bary.u * (c0.b * invZ0) + bary.v * (c1.b * invZ1) + bary.w * (c2.b * invZ2)) / invZ;
                        float w0 = bary.u * invZ0;
                        float w1 = bary.v * invZ1;
                        float w2 = bary.w * invZ2;
                        float weight = w0 + w1 + w2;
                        Vector3f worldPos = (t.getVertex(0) * w0 + t.getVertex(1) * w1 + t.getVertex(2) * w2) * (1.0f / weight);
                        if (shadowMap && shadowMapSize > 0 && lightCam) {
                            if (isInShadow(depth, *shadowMap, shadowMapSize, *lightCam, worldPos)) {
                                r *= 0.3f;
                                g *= 0.3f;
                                b *= 0.3f;
                            }
                        }
                        sf::Color col(
                            static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, r))),
                            static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, g))),
                            static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, b)))
                        );
                        colorBuffer.setPixel(sf::Vector2u(x, y), col);
                        depthBuffer[index] = depth;
                    }
                } else {
                    float depth = bary.u * z0 + bary.v * z1 + bary.w * z2;
                    if (depth < depthBuffer[index]) {
                        float r = bary.u * c0.r + bary.v * c1.r + bary.w * c2.r;
                        float g = bary.u * c0.g + bary.v * c1.g + bary.w * c2.g;
                        float b = bary.u * c0.b + bary.v * c1.b + bary.w * c2.b;
                        Vector3f worldPos = t.getVertex(0) * bary.u + t.getVertex(1) * bary.v + t.getVertex(2) * bary.w;
                        if (shadowMap && shadowMapSize > 0 && lightCam) {
                            if (isInShadow(depth, *shadowMap, shadowMapSize, *lightCam, worldPos)) {
                                r *= 0.3f;
                                g *= 0.3f;
                                b *= 0.3f;
                            }
                        }
                        sf::Color col(
                            static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, r))),
                            static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, g))),
                            static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, b)))
                        );
                        colorBuffer.setPixel(sf::Vector2u(x, y), col);
                        depthBuffer[index] = depth;
                    }
                }
            }
        }
    }
}
