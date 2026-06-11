#pragma once

#include "vmath.h"
#include <vector>

class Light {

public:

    Vector3f direction;
    Vector3f position;

    Light(Vector3f dir) : direction(dir.normalized()) {

        position = -direction * 5.0f;
    }



    struct LightCamera {
        Vector3f position;
        Vector3f target;
        Vector3f up;
        float orthoSize;

        Vector3f project(Vector3f p, int shadowMapSize) const {

            Vector3f forward = (target - position).normalized();
            Vector3f right = vm::cross(up, forward).normalized();
            Vector3f camUp = vm::cross(forward, right);

            Vector3f rel = p - position;

            float x = vm::dot(right, rel);
            float y = vm::dot(camUp, rel);
            float zView = vm::dot(forward, rel);

            if (zView <= 0) return {-1, -1, -1};

            float halfSize = orthoSize / 2.0f;
            float ndcX = x / halfSize;
            float ndcY = y / halfSize;

            float screenX = (ndcX + 1.0f) * 0.5f * shadowMapSize;
            float screenY = (1.0f - ndcY) * 0.5f * shadowMapSize;

            return {screenX, screenY, zView};
        }
    };

    LightCamera getLightCamera() const {
        LightCamera cam;
        cam.position = position;
        cam.target = {0, 0, 0};
        cam.up = {0, 1, 0};
        cam.orthoSize = 10.0f;
        return cam;
    }

};
