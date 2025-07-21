#pragma once
#include "Common/Math/Vector2f.h"

class Vector2f_NetQuantize : Vector2f
{
	Vector2f_NetQuantize() : Vector2f(0.0f, 0.0f) {}

	Vector2f_NetQuantize(const float X, const float Y) : Vector2f(X, Y) {}

	Vector2f_NetQuantize(const Vector2f& Vector2f)
	{
		Vector2f::operator=(Vector2f);
	}

	bool NetSerialize();
};

