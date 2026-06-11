#include "transform.h"

Matrix4f Transform::getMatrix() const {

    Matrix4f trans = vm::translation(position.x, position.y, position.z);

    Matrix4f rotX = vm::rotationX(rotation.x);

    Matrix4f rotY = vm::rotationY(rotation.y);

    Matrix4f rotZ = vm::rotationZ(rotation.z);

    Matrix4f scl = vm::scale(scale.x, scale.y, scale.z);



    return trans * rotZ * rotY * rotX * scl;

}
