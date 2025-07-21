#include "Common/Math/Vector2f_NetQuantize.h"
#include <cmath>
#include <algorithm>

bool Vector2f_NetQuantize::NetSerialize()
{
	Vector2f value = { x ,y };

	// 소수점 1자리까지
	value *= 10;

	if (value.ContationNaN()) return false;

	const Vector2f ClampedValue = ClampVector(value, Vector2f(-1073741824.0f, -1073741824.0f), Vector2f(1073741760.0f, 1073741760.0f));
	bool isClamp = ClampedValue != value;

	long lx = std::lround(ClampedValue.x);
	long ly = std::lround(ClampedValue.y);

	uint32_t bits = std::clamp<uint32_t>(std::ceil(1 + std::max(std::abs(lx), std::abs(ly))), 1, 24) - 1;


	return true;
}
