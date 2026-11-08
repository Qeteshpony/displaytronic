#include "debug.h"

static bool debug_enabled = true;

void debugInit(uint32_t baudrate)
{
    Serial.begin(baudrate);
}

void debugSetEnabled(bool enabled)
{
    debug_enabled = enabled;
}

bool debugIsEnabled()
{
    return debug_enabled;
}
