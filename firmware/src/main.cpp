#include <Arduino.h>
#include "watchdog.h"
#include "pin_init.h"
#include "flipdot.h"
#include "debug.h"
#include "i2c_interface.h"
#include "voltage.h"

static Display display;

void setup() {
    debugInit(500000);
    DEBUG("Debugging is enabled");
    DEBUG("Reboot Reason: 0x%x", wdtGetBootReason());
    pinInit();
    i2cInit(display);  // the display is initialized via I2C
    wdtEnable();
    DEBUG("Watchdog is enabled.");
}

static uint32_t lastVoltageMeasurement_ = 0;

void loop() {
    wdtFeed();
    i2cLoop();

    if (millis() - lastVoltageMeasurement_ > 1000)
    {
        lastVoltageMeasurement_ = millis();
        measureVoltages();
    }
}