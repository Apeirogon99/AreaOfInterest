#pragma once
#include <SDL.h>
#include <Common/Math/Vector2f.h>

class SDL2Utils
{
public:
	static void SDL_SetRenderDrawCircleF(SDL_Renderer* Renderer, const Vector2f& Center, float Radius);
};
