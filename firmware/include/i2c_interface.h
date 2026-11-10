#pragma once
#include <stdint.h>
#include "flipdot.h"


constexpr uint8_t I2C_ADDRESS = 0x30;

void i2cInit(Display& display);
void i2cEnable();
void i2cDisable();
void i2cLoop();


