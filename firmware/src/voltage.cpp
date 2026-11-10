#include <util/atomic.h>
#include "voltage.h"
#include "pins.h"

// Voltage measurement
// -------------------
// ADC: 10 bit (0..1023), reference AREF = 2.5 V (external, analogReference(EXTERNAL))
//   U_adc = ADC * 2500 mV / 1024
//
// Voltage dividers (R_top to input voltage, R_bottom = 10k to GND, ADC measures across R_bottom):
//   U_in = U_adc * (R_top + R_bottom) / R_bottom
//
// Logic supply:   R_top = 15k  -> factor (15k + 10k) / 10k  = 2.5
//   U_in = ADC * 2500 * 2.5 / 1024 = ADC * 6250 / 1024 = ADC * 3125 / 512
//   -> mV = (ADC * 3125) >> 9                 (max. ~6244 mV)
//
// Flip-dot supply: R_top = 100k -> factor (100k + 10k) / 10k = 11
//   U_in = ADC * 2500 * 11 / 1024 = ADC * 27500 / 1024 = ADC * 6875 / 256
//   -> mV = (ADC * 6875) >> 8                 (max. ~27473 mV)
//
// The fractions are reduced to a power-of-two denominator so the division
// becomes a bit shift and half the denominator is added before shifting to round instead of truncate
// (+256 for >> 9, +128 for >> 8).

static volatile uint16_t voltage_logic_;
static volatile uint16_t voltage_flipdot_;

void measureVoltages()
{
    uint16_t voltage_logic = (analogRead(PIN_MEASURE_LOGIC) * 3125UL + 256) >> 9;
    uint16_t voltage_flipdot = (analogRead(PIN_MEASURE_FLIPDOT) * 6875UL + 128) >> 8;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        voltage_logic_ = voltage_logic;
        voltage_flipdot_ = voltage_flipdot;
    }
}

uint16_t getVoltageLogic()
{
    return voltage_logic_;
}

uint16_t getVoltageFlipdot()
{
    return voltage_flipdot_;
}


