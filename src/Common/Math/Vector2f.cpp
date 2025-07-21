#include "Common/Math/Vector2f.h"
#include <cmath>
#include <algorithm>

Vector2f Lerp(const Vector2f& a, const Vector2f& b, float t)
{
    return a + (b - a) * t;
}

Vector2f ClampVector(const Vector2f& Value, const Vector2f& Min, const Vector2f& Max)
{
    return Vector2f(std::clamp(Value.x, Min.x, Max.x), std::clamp(Value.y, Min.y, Max.y));
}