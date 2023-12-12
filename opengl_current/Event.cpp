#include "Event.h"

Event::Event()
{
    memset(this, 0, sizeof(*this));
    MillisecondsSinceGameStart = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
}