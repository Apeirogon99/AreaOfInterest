#pragma once
#include <cmath>

class Vector2f
{
public:
    Vector2f() : x(0.0f), y(0.0f) {}
    Vector2f(float X, float Y) : x(X), y(Y) {}
    Vector2f(const Vector2f& Other) : x(Other.x), y(Other.y) {}

public:

    Vector2f operator+(const Vector2f& other) const 
    {
        return Vector2f(x + other.x, y + other.y);
    }

    Vector2f operator-(const Vector2f& other) const 
    {
        return Vector2f(x - other.x, y - other.y);
    }

    Vector2f operator*(float scalar) const 
    {
        return Vector2f(x * scalar, y * scalar);
    }

    Vector2f operator/(float scalar) const 
    {
        if (scalar != 0.0f) 
        {
            return Vector2f(x / scalar, y / scalar);
        }
        return Vector2f(0.0f, 0.0f);
    }

    Vector2f& operator+=(const Vector2f& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vector2f& operator-=(const Vector2f& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vector2f& operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vector2f& operator/=(float scalar)
    {
        if (scalar != 0.0f)
        {
            x /= scalar;
            y /= scalar;
        }
        return *this;
    }

    bool operator==(const Vector2f& other) const 
    {
        const float epsilon = 1e-6f;
        return std::abs(x - other.x) < epsilon && std::abs(y - other.y) < epsilon;
    }

    bool operator!=(const Vector2f& other) const 
    {
        return !(*this == other);
    }

public:

    float magnitude() const 
    {
        return std::sqrt(x * x + y * y);
    }

    float magnitudeSquared() const 
    {
        return x * x + y * y;
    }

    Vector2f normalized() const 
    {
        float mag = magnitude();
        if (mag > 0.0f) 
        {
            return Vector2f(x / mag, y / mag);
        }
        return Vector2f(0.0f, 0.0f);
    }

    void normalize() 
    {
        float mag = magnitude();
        if (mag > 0.0f) 
        {
            x /= mag;
            y /= mag;
        }
    }

    float dot(const Vector2f& other) const 
    {
        return x * other.x + y * other.y;
    }

    float cross(const Vector2f& other) const 
    {
        return x * other.y - y * other.x;
    }

    float distance(const Vector2f& other) const 
    {
        return (*this - other).magnitude();
    }

    float distanceSquared(const Vector2f& other) const 
    {
        return (*this - other).magnitudeSquared();
    }

    float angle() const 
    {
        return std::atan2(y, x);
    }

    bool isZero() const 
    {
        const float epsilon = 1e-6f;
        return std::abs(x) < epsilon && std::abs(y) < epsilon;
    }

    bool ContationNaN()
    {
        return !isfinite(x) || !isfinite(y);
    }

public:
    float x;
    float y;
};

Vector2f Lerp(const Vector2f& a, const Vector2f& b, float t);
Vector2f ClampVector(const Vector2f& Value, const Vector2f& Min, const Vector2f& Max);