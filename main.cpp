#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <algorithm>
#include <vector>
#include <limits>
#include <string>
#include <filesystem>
#include <iostream>

#include "vmath.h"
#include "camera.h"
#include "mesh.h"
#include "primitives.h"
#include "debug.h"
#include "input.h"
#include "light.h"
#include "import.h"

namespace fs = std::filesystem;

static std::string findObjectsFolderPath()
{
    std::vector<fs::path> candidates;
    candidates.push_back(fs::current_path());

    fs::path path = fs::current_path();
    for (int i = 0; i < 5; ++i) {
        path = path.parent_path();
        if (path.empty())
            break;
        candidates.push_back(path);
    }

    fs::path sourceRoot = fs::path(__FILE__).parent_path();
    candidates.push_back(sourceRoot);

    for (const auto& candidate : candidates) {
        fs::path folder = candidate / "objects";
        if (fs::exists(folder) && fs::is_directory(folder)) {
            return folder.string();
        }
    }

    return "./objects";
}

int main()
{
    int width = 800;
    int height = 600;

    sf::RenderWindow window(sf::VideoMode({(unsigned int)width, (unsigned int)height}), "SFML 3 Window", sf::Style::Resize);

    Camera cam;
    cam.position = {2,2,2};
    cam.target   = {0,0,0};
    cam.up       = {0,1,0};
    cam.fov      = 90.0f * 3.14159265f / 180.0f;
    cam.aspectRatio = (float)width / height;
    cam.nearPlane = 0.1f;
    cam.farPlane  = 100.f;
    cam.isPerspective = true;
    cam.orthoSize = 4.0f;
    float radius = cam.position.length();
    float yaw   = std::atan2(cam.position.z, cam.position.x);
    float pitch = std::asin(cam.position.y / radius);

    Light light({0, -1, -1});

    sf::Clock clock;

    sf::Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {

    }

    DebugState debugState;
    bool tabLast = false;
    bool num1Last = false, num2Last = false, num3Last = false, num4Last = false;

    std::vector<Mesh> importedMeshes;


    std::string objectsFolder = findObjectsFolderPath();
    std::cerr << "Importing meshes from: " << objectsFolder << "\n";
    importedMeshes = importMeshesFromFolder(objectsFolder);
    if (importedMeshes.empty()) {
        std::cerr << "No imported meshes found in " << objectsFolder << "\n";
    }

    std::vector<Mesh> primitiveMeshes;
    primitiveMeshes.push_back(createIcosphere(0.7f, 2, sf::Color::White, Transform({0, 1.5f, 1.5f}, {0,0,0}, {1,1,1})));
    primitiveMeshes.push_back(createCube(1.0f, sf::Color::Red, Transform({0, 0, 0}, {0,0,0}, {1,1,1})));

    sf::Image colorBuffer(sf::Vector2u(width, height), sf::Color::Blue);
    std::vector<float> depthBuffer;

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        debugState.fps = 1.0f / dt;

        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (event->is<sf::Event::Resized>())
            {
                sf::Vector2u size = window.getSize();
                width = size.x;
                height = size.y;
                cam.aspectRatio = (float)width / height;
            }
        }

        handleInput(debugState, cam, yaw, pitch, radius, dt, window, tabLast, num1Last, num2Last, num3Last, num4Last);

        window.clear();
        colorBuffer = sf::Image(sf::Vector2u(width, height), sf::Color(200,200,200));
        depthBuffer.assign(width * height, 1e10f);


        int shadowMapSize = 512;
        std::vector<float> shadowDepthBuffer(shadowMapSize * shadowMapSize, 1e10f);
        Light::LightCamera lightCam = light.getLightCamera();
        const std::vector<Mesh>& meshesToRender = debugState.showImportedObjects ? importedMeshes : primitiveMeshes;

        for (auto& mesh : meshesToRender) {
            mesh.rasterizeShadow(shadowDepthBuffer, lightCam, shadowMapSize);
        }


        for (auto& mesh : meshesToRender) {
            mesh.rasterize(colorBuffer, depthBuffer, cam, width, height, light, &lightCam, &shadowDepthBuffer, shadowMapSize);
        }


        float minDepth = std::numeric_limits<float>::max();
        float maxDepth = -std::numeric_limits<float>::max();
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                float d = depthBuffer[y * width + x];
                if (d < 1e10f) {
                    minDepth = std::min(minDepth, d);
                    maxDepth = std::max(maxDepth, d);
                }
            }
        }


        float shadowMinDepth = std::numeric_limits<float>::max();
        float shadowMaxDepth = -std::numeric_limits<float>::max();
        for (int y = 0; y < shadowMapSize; y++) {
            for (int x = 0; x < shadowMapSize; x++) {
                float d = shadowDepthBuffer[y * shadowMapSize + x];
                if (d < 1e10f) {
                    shadowMinDepth = std::min(shadowMinDepth, d);
                    shadowMaxDepth = std::max(shadowMaxDepth, d);
                }
            }
        }


        if (debugState.viewMode == 1) {

            sf::Image depthImage(sf::Vector2u(width, height), sf::Color::Black);
            if (minDepth < maxDepth) {
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        float depth = depthBuffer[y * width + x];
                        if (depth < 1e10f) {
                            float normalized = (depth - minDepth) / (maxDepth - minDepth);
                            normalized = std::clamp(normalized, 0.0f, 1.0f);
                            uint8_t gray = static_cast<uint8_t>(255 * (1.0f - normalized));
                            depthImage.setPixel(sf::Vector2u(x, y), sf::Color(gray, gray, gray));
                        }
                    }
                }
            } else {

                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        if (depthBuffer[y * width + x] < 1e10f) {
                            depthImage.setPixel(sf::Vector2u(x, y), sf::Color(128, 128, 128));
                        }
                    }
                }
            }
            sf::Texture depthTexture;
            if (depthTexture.loadFromImage(depthImage)) {
                sf::Sprite depthSprite(depthTexture);
                window.draw(depthSprite);
            }
            for (const auto& mesh : meshesToRender) {
                mesh.drawWireframe(window, cam, width, height);
            }
        } else if (debugState.viewMode == 2) {

            sf::Image shadowMapImage(sf::Vector2u(shadowMapSize, shadowMapSize), sf::Color::Black);
            if (shadowMinDepth < shadowMaxDepth) {
                for (int y = 0; y < shadowMapSize; y++) {
                    for (int x = 0; x < shadowMapSize; x++) {
                        float depth = shadowDepthBuffer[y * shadowMapSize + x];
                        if (depth < 1e10f) {
                            float normalized = (depth - shadowMinDepth) / (shadowMaxDepth - shadowMinDepth);
                            normalized = std::clamp(normalized, 0.0f, 1.0f);
                            uint8_t gray = static_cast<uint8_t>(255 * (1.0f - normalized));
                            shadowMapImage.setPixel(sf::Vector2u(x, y), sf::Color(gray, gray, gray));
                        }
                    }
                }
            } else {

                for (int y = 0; y < shadowMapSize; y++) {
                    for (int x = 0; x < shadowMapSize; x++) {
                        if (shadowDepthBuffer[y * shadowMapSize + x] < 1e10f) {
                            shadowMapImage.setPixel(sf::Vector2u(x, y), sf::Color(128, 128, 128));
                        }
                    }
                }
            }
            sf::Texture shadowTexture;
            if (shadowTexture.loadFromImage(shadowMapImage)) {
                sf::Sprite shadowSprite(shadowTexture);
                window.draw(shadowSprite);
            }
        } else {

            sf::Texture texture;
            if (texture.loadFromImage(colorBuffer)) {
                sf::Sprite sprite(texture);
                window.draw(sprite);
            }
        }

        drawDebugOverlay(window, debugState, font);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Tab)) {
            Vector3f lightScreen = cam.project(light.position, width, height);
            if (lightScreen.z > 0) {
                sf::CircleShape lightMarker(6.0f);
                lightMarker.setOrigin(sf::Vector2f{6.0f, 6.0f});
                lightMarker.setPosition(sf::Vector2f{lightScreen.x, lightScreen.y});
                lightMarker.setFillColor(sf::Color::Transparent);
                lightMarker.setOutlineColor(sf::Color::Red);
                lightMarker.setOutlineThickness(2.0f);
                window.draw(lightMarker);
            }
        }

        window.display();
    }
}
