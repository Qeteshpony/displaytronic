#include <Arduino.h>

#include "pin_init.h"
#include "pins.h"

void initPins()
{
    pinMode(PIN_STATUS_LED, OUTPUT);
    pinMode(PIN_LED_PWM, OUTPUT);
    pinMode(PIN_HIGH_LOW, OUTPUT);
    pinMode(PIN_PULSE, OUTPUT);

    pinMode(PIN_BOARDSEL_0, OUTPUT);
    pinMode(PIN_BOARDSEL_1, OUTPUT);
    pinMode(PIN_BOARDSEL_2, OUTPUT);

    pinMode(PIN_ROWSEL_0, OUTPUT);
    pinMode(PIN_ROWSEL_1, OUTPUT);
    pinMode(PIN_ROWSEL_2, OUTPUT);
    pinMode(PIN_ROWSEL_3, OUTPUT);

    pinMode(PIN_SEGSEL_0, OUTPUT);
    pinMode(PIN_SEGSEL_1, OUTPUT);

    pinMode(PIN_COLSEL_0, OUTPUT);
    pinMode(PIN_COLSEL_1, OUTPUT);
    pinMode(PIN_COLSEL_2, OUTPUT);

    pinMode(PIN_MEASURE_5V, INPUT);
    pinMode(PIN_MEASURE_24V, INPUT);
}
