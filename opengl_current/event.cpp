#include "event.h"

Event::Event()
{
    memset(this, 0, sizeof(*this));
    time_from_game_start = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
}