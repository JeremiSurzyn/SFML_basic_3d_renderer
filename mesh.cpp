#include "mesh.h"
#include "triangle.h"
#include <algorithm>
#include <map>
#include <tuple>

bool isFacingCamera(const Triangle& tri, const Camera& cam)
{
    Vector3f center = (tri.getVertex(0) + tri.getVertex(1) + tri.getVertex(2)) * (1.0f / 3.0f);
    Vector3f viewDir = cam.position - center;
    return vm::dot(tri.getNormal(), viewDir) > 0;
}

void Mesh::rasterize(sf::Image& colorBuffer, std::vector<float>& depthBuffer, const Camera& cam, int width, int height, const Light& light, const Light::LightCamera* lightCam, const std::vector<float>* shadowMap, int shadowMapSize) const {
    if (!shadeSmooth) {
        for (auto& triPtr : triangles) {
            Triangle transformed = *triPtr;
            transformed.transform(transform.getMatrix());
            if (isFacingCamera(transformed, cam)) {

                Vector3f normal = transformed.getNormal();
                float dot = vm::dot(normal, -light.direction);
                float intensity = std::max(0.0f, dot);
                sf::Color shadedColor = sf::Color(
                    static_cast<uint8_t>(color.r * intensity),
                    static_cast<uint8_t>(color.g * intensity),
                    static_cast<uint8_t>(color.b * intensity)
                );
                Triangle::rasterise(colorBuffer, depthBuffer, transformed, cam, width, height, shadedColor, shadowMap, shadowMapSize, lightCam);
            }
        }
        return;
    }



    std::map<std::tuple<float,float,float>, Vector3f> normalSum;
    std::map<std::tuple<float,float,float>, int> normalCount;


    for (auto& triPtr : triangles) {
        Triangle transformed = *triPtr;
        transformed.transform(transform.getMatrix());
        if (!isFacingCamera(transformed, cam)) continue;
        Vector3f triNormal = transformed.getNormal();
        for (int i = 0; i < 3; ++i) {
            Vector3f v = transformed.getVertex(i);
            auto key = std::make_tuple(v.x, v.y, v.z);
            if (normalSum.find(key) == normalSum.end()) normalSum[key] = {0,0,0};
            normalSum[key] = normalSum[key] + triNormal;
            normalCount[key] += 1;
        }
    }


    std::map<std::tuple<float,float,float>, Vector3f> vertexNormal;
    for (auto& kv : normalSum) {
        auto key = kv.first;
        Vector3f sum = kv.second;
        int cnt = normalCount[key];
        if (cnt > 0) sum = sum * (1.0f / static_cast<float>(cnt));
        vertexNormal[key] = sum.normalized();
    }


    for (auto& triPtr : triangles) {
        Triangle transformed = *triPtr;
        transformed.transform(transform.getMatrix());
        if (!isFacingCamera(transformed, cam)) continue;

        sf::Color vcol[3];
        for (int i = 0; i < 3; ++i) {
            Vector3f v = transformed.getVertex(i);
            auto key = std::make_tuple(v.x, v.y, v.z);
            Vector3f n = {0,0,0};
            auto it = vertexNormal.find(key);
            if (it != vertexNormal.end()) n = it->second;
            float dot = vm::dot(n, -light.direction);
            float intensity = std::max(0.0f, dot);
            vcol[i] = sf::Color(
                static_cast<uint8_t>(std::min(255.0f, color.r * intensity)),
                static_cast<uint8_t>(std::min(255.0f, color.g * intensity)),
                static_cast<uint8_t>(std::min(255.0f, color.b * intensity))
            );
        }
        Triangle::rasterise(colorBuffer, depthBuffer, transformed, cam, width, height, vcol[0], vcol[1], vcol[2], shadowMap, shadowMapSize, lightCam);
    }
}

void Mesh::rasterizeShadow(std::vector<float>& shadowDepthBuffer, const Light::LightCamera& lightCam, int shadowMapSize) const {
    for (auto& triPtr : triangles) {
        Triangle transformed = *triPtr;
        transformed.transform(transform.getMatrix());
        Triangle::rasteriseShadow(shadowDepthBuffer, transformed, lightCam, shadowMapSize);
    }
}

void Mesh::drawWireframe(sf::RenderWindow& window, const Camera& cam, int width, int height) const {
    for (auto& triPtr : triangles) {
        Triangle transformed = *triPtr;
        transformed.transform(transform.getMatrix());
        Triangle::drawWireframe(window, transformed, cam, width, height, color);
    }
}
