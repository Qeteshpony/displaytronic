#include <Arduino.h>

#include "pin_init.h"
#include "pins.h"
#include "flipdot.h"
#include "debug.h"

static Display display;

void setup() {
    debugInit(115200);
    DEBUG("Debugging is enabled");
    initPins();
    display.init(1);
}

void loop() {
}