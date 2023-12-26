#include "event.h"

Event::Event() {
    memset(this, 0, sizeof(*this));
    milliseconds_since_game_start = static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count());
}