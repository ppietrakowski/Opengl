#include "sandbox_game_layer.h"

int32_t main()
{
    Game game(WindowSettings{1280, 720, "Game"});

    game.AddLayer(std::make_unique<SandboxGameLayer>());
    game.SetMouseVisible(false);

    game.Run();

    return EXIT_SUCCESS;
}

