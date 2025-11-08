#pragma once
#include <cmath>

struct Vector2 {
    float x, y;

    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) : x(x), y(y) {}
};

struct Vector3 {
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    float Distance(const Vector3& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        float dz = z - other.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    Vector3 operator+(const Vector3& v) const {
        return Vector3(x + v.x, y + v.y, z + v.z);
    }

    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }
};

struct Matrix {
    float matrix[16];

    bool IsValid() const {
        // Vérification simple de validité
        return matrix[0] != 0.0f || matrix[5] != 0.0f;
    }
};

inline Vector3 WorldToScreen(const Vector3& pos, const Matrix& matrix, const Vector2& screenSize) {
    Vector3 out;

    float w = matrix.matrix[12] * pos.x +
        matrix.matrix[13] * pos.y +
        matrix.matrix[14] * pos.z +
        matrix.matrix[15];

    if (w < 0.01f) {
        out.z = -1.0f;
        return out;
    }

    out.z = w;

    float x = matrix.matrix[0] * pos.x +
        matrix.matrix[1] * pos.y +
        matrix.matrix[2] * pos.z +
        matrix.matrix[3];

    float y = matrix.matrix[4] * pos.x +
        matrix.matrix[5] * pos.y +
        matrix.matrix[6] * pos.z +
        matrix.matrix[7];

    out.x = (screenSize.x / 2.0f) + (screenSize.x / 2.0f) * x / w;
    out.y = (screenSize.y / 2.0f) - (screenSize.y / 2.0f) * y / w;

    return out;
}