#include <Engine.h>

#include "SandboxGameLayer.h"

int main()
{
    Game game(WindowSettings{ 1280, 720, "Game" });
    game.AddLayer(new SandboxGameLayer());

    game.Run();

    return EXIT_SUCCESS;
}

