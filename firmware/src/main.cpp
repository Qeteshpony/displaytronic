#include <Arduino.h>

#include "pin_init.h"
#include "pins.h"
#include "flipdot.h"

static Display display;

void setup() {
    initPins();
    display.init(1);
}

void loop() {
}