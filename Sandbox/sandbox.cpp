#include "sandbox_game_layer.h"

int main()
{
    Game game(WindowSettings{1440, 810, "Game"});
    game.SetMouseVisible(false);
     
    game.AddLayer(std::make_unique<SandboxGameLayer>(&game));
    game.Run();

    return EXIT_SUCCESS;
}

