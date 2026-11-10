#include <Arduino.h>
#include <util/atomic.h>
#include "light.h"
#include "pins.h"

static volatile uint16_t brightness = 0;

static void setPWM()
{
    if (brightness > 0)
    {
        OCR1A = brightness;
        TCCR1A = (1 << COM1A1) | (1 << WGM10) | (1 << WGM11);
        TCCR1B = (1 << WGM12) | (1 << CS10);
    }
    else
    {
        TCCR1A &= ~(1 << COM1A1);
        digitalWrite(PIN_LED_PWM, LOW);
    }
}

void lightSetBrightness256(uint8_t value)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        brightness = (value << 2) | (value >> 6);
        setPWM();
    }
}

void lightSetBrightness1024(uint16_t value)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if (value > 1023) brightness = 1023;
        else brightness = value;
        setPWM();
    }
}

uint16_t lightGetBrightness1024()
{
    uint16_t value = 0;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        value = brightness;
    }
    return value;
}

uint8_t lightGetBrightness256()
{
    return lightGetBrightness1024() >> 2;
}
