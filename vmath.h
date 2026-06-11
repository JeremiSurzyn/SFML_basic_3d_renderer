#pragma once
#include <cmath>

struct Vector3f {
    float x, y, z;

    Vector3f operator+(const Vector3f& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }
    Vector3f operator-(const Vector3f& other) const {
        return { x - other.x, y - other.y, z - other.z };
    }
    Vector3f operator*(float scalar) const {
        return { x * scalar, y * scalar, z * scalar };
    }

    Vector3f operator-() const {
        return { -x, -y, -z };
    }

    float length() const {
        return std::sqrt(x*x + y*y + z*z);
    }
    float lengthSquared() const {
        return x*x + y*y + z*z;
    }
    Vector3f normalized() const {
        float len = length();
        if (len == 0.0f) return {0,0,0};
        return { x/len, y/len, z/len };
    }
};

struct Matrix4f {
    float m[4][4];

    Matrix4f() {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                m[i][j] = (i == j) ? 1.0f : 0.0f;
    }

    Matrix4f operator*(const Matrix4f& other) const {
        Matrix4f result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    result.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        return result;
    }

    Vector3f operator*(const Vector3f& v) const {
        float x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3];
        float y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3];
        float z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3];
        float w = m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3];
        if (w != 0.0f) {
            x /= w;
            y /= w;
            z /= w;
        }
        return {x, y, z};
    }
};

namespace vm {
    inline float dot(const Vector3f& a, const Vector3f& b) {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    }
    inline Vector3f cross(const Vector3f& a, const Vector3f& b) {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    inline Matrix4f translation(float x, float y, float z) {
        Matrix4f mat;
        mat.m[0][3] = x;
        mat.m[1][3] = y;
        mat.m[2][3] = z;
        return mat;
    }

    inline Matrix4f rotationX(float angle) {
        Matrix4f mat;
        float c = cos(angle);
        float s = sin(angle);
        mat.m[1][1] = c; mat.m[1][2] = -s;
        mat.m[2][1] = s; mat.m[2][2] = c;
        return mat;
    }

    inline Matrix4f rotationY(float angle) {
        Matrix4f mat;
        float c = cos(angle);
        float s = sin(angle);
        mat.m[0][0] = c; mat.m[0][2] = s;
        mat.m[2][0] = -s; mat.m[2][2] = c;
        return mat;
    }

    inline Matrix4f rotationZ(float angle) {
        Matrix4f mat;
        float c = cos(angle);
        float s = sin(angle);
        mat.m[0][0] = c; mat.m[0][1] = -s;
        mat.m[1][0] = s; mat.m[1][1] = c;
        return mat;
    }

    inline Matrix4f scale(float x, float y, float z) {
        Matrix4f mat;
        mat.m[0][0] = x;
        mat.m[1][1] = y;
        mat.m[2][2] = z;
        return mat;
    }
}
