#include "SDL2Utils.h"

void SDL2Utils::SDL_SetRenderDrawCircleF(SDL_Renderer* Renderer, const Vector2f& Center, float Radius)
{
    float x = 0;
    float y = Radius;
    float decision = 3 - 2 * Radius;

    while (x <= y)
    {
        SDL_RenderDrawPointF(Renderer, Center.x + x, Center.y + y);
        SDL_RenderDrawPointF(Renderer, Center.x - x, Center.y + y);
        SDL_RenderDrawPointF(Renderer, Center.x + x, Center.y - y);
        SDL_RenderDrawPointF(Renderer, Center.x - x, Center.y - y);
        SDL_RenderDrawPointF(Renderer, Center.x + y, Center.y + x);
        SDL_RenderDrawPointF(Renderer, Center.x - y, Center.y + x);
        SDL_RenderDrawPointF(Renderer, Center.x + y, Center.y - x);
        SDL_RenderDrawPointF(Renderer, Center.x - y, Center.y - x);

        if (decision < 0) 
        {
            decision += 4 * x + 6;
        }
        else 
        {
            decision += 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}
