#include "vmath.h"
#include "camera.h"
#include <cmath>

Vector3f Camera::project(Vector3f p, int screenWidth, int screenHeight) const
{

    Vector3f forward = (target - position).normalized();
    Vector3f right = vm::cross(up,forward).normalized();
    Vector3f camUp = vm::cross(forward,right);


    Vector3f rel = p - position;

    float x = vm::dot(right,rel);
    float y = vm::dot(camUp,rel);
    float zView = vm::dot(forward,rel);


    if (zView <= 0) return {-1,-1,-1};

    float z = rel.length();

    float ndcX, ndcY;

    if (isPerspective) {

        float f = 1.0f / tan(fov * 0.5f);

        ndcX = (x * f / aspectRatio) / zView;
        ndcY = (y * f) / zView;
    } else {

        float orthoHalfHeight = orthoSize / 2.0f;
        float orthoHalfWidth = orthoHalfHeight * aspectRatio;

        ndcX = x / orthoHalfWidth;
        ndcY = y / orthoHalfHeight;
    }


    float screenX = (ndcX + 1.0f) * 0.5f * screenWidth;
    float screenY = (1.0f - ndcY) * 0.5f * screenHeight;

    return {screenX, screenY, zView};
}

