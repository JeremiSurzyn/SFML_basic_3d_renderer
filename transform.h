#pragma once

#include "vmath.h"

class Transform {

public:

    Vector3f position;

    Vector3f rotation;

    Vector3f scale;

    Transform(Vector3f pos = {0,0,0}, Vector3f rot = {0,0,0}, Vector3f scl = {1,1,1}) : position(pos), rotation(rot), scale(scl) {}

    Matrix4f getMatrix() const;

};
