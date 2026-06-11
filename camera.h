#pragma once
#include "vmath.h"
class Camera{
    public:
        Vector3f position;
        Vector3f target;
        Vector3f up;
        float fov;
        float aspectRatio;
        float nearPlane;
        float farPlane;
        bool isPerspective;
        float orthoSize;
    public:
        Vector3f project(Vector3f p, int screenWidth, int screenHeight) const;
};
