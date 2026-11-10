#pragma once

#include <Arduino.h>

void wdtEnable();
void wdtDisable();
void wdtFeed();
uint8_t wdtGetBootReason();



