#include "Game.h"

int main(void)
{
    std::unique_ptr<Game> game = std::make_unique<Game>();

    if (game->Initialize())
    {
        game->Run();
    }

    game->Destroy();
    return 0;
}