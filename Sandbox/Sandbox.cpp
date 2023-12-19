#include <Engine.h>

#include "SandboxGameLayer.h"

int main()
{
    Game game(WindowSettings{ 1280, 720, "Game" });
    try
    {
        game.AddLayer(new SandboxGameLayer());
        game.SetMouseVisible(false);

        game.Run();
    }
    catch (const std::exception& e)
    {
        ELOG_ERROR(LOG_CORE, "%s", e.what());
    }
    catch (...)
    {
        ELOG_ERROR(LOG_CORE, "XD");
    }

    return EXIT_SUCCESS;
}

