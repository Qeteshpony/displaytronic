#include <Arduino.h>

#include "pin_init.h"
#include "flipdot.h"
#include "debug.h"
#include "i2c_interface.h"
#include "voltage.h"

static Display display;

void setup() {
    debugInit(500000);
    DEBUG("Debugging is enabled");

    pinInit();
    i2cInit(display);  // the display is initialized via I2C
}

static uint32_t lastVoltageMeasurement_ = 0;

void loop() {
    i2cLoop();

    if (millis() - lastVoltageMeasurement_ > 1000)
    {
        lastVoltageMeasurement_ = millis();
        measureVoltages();
    }
}