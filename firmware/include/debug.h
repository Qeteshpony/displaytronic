
#pragma once
#include <Arduino.h>

void debugInit(uint32_t baudrate);
void debugSetEnabled(bool enabled);
bool debugIsEnabled();

#ifdef DEBUGGING

#define DEBUG(message, ...) do { if (debugIsEnabled()) Serial.printf(F(message), ##__VA_ARGS__); } while (0)

#else

#define DEBUG(message, ...) do {} while (0)

#endif
