// define avr pins

#pragma once
#include <Arduino.h>

// funktion to atmega pin

constexpr uint8_t PIN_STATUS_LED  = PIN_PB0;
constexpr uint8_t PIN_LED_PWM     = PIN_PB1;
constexpr uint8_t PIN_HIGH_LOW    = PIN_PB2;
constexpr uint8_t PIN_BOARDSEL_2 = PIN_PB3;
constexpr uint8_t PIN_BOARDSEL_1 = PIN_PB4;
constexpr uint8_t PIN_BOARDSEL_0 = PIN_PB5;

constexpr uint8_t PIN_ROWSEL_0    = PIN_PC0;
constexpr uint8_t PIN_ROWSEL_1    = PIN_PC1;
constexpr uint8_t PIN_ROWSEL_2    = PIN_PC2;
constexpr uint8_t PIN_ROWSEL_3    = PIN_PC3;
constexpr uint8_t PIN_SDA         = PIN_PC4;
constexpr uint8_t PIN_SCL         = PIN_PC5;

constexpr uint8_t PIN_RXD         = PIN_PD0;
constexpr uint8_t PIN_TXD         = PIN_PD1;
constexpr uint8_t PIN_PULSE       = PIN_PD2;
constexpr uint8_t PIN_SEGSEL_1    = PIN_PD3;
constexpr uint8_t PIN_SEGSEL_0    = PIN_PD4;
constexpr uint8_t PIN_COLSEL_2    = PIN_PD5;
constexpr uint8_t PIN_COLSEL_1    = PIN_PD6;
constexpr uint8_t PIN_COLSEL_0    = PIN_PD7;

constexpr uint8_t PIN_MEASURE_5V  = PIN_A6;
constexpr uint8_t PIN_MEASURE_24V = PIN_A7;

// pin groups

constexpr uint8_t PINS_BOARD[] = {PIN_BOARDSEL_0, PIN_BOARDSEL_1, PIN_BOARDSEL_2};
constexpr uint8_t PINS_SEGMENT[] = {PIN_SEGSEL_0, PIN_SEGSEL_1};
constexpr uint8_t PINS_COLUMN[] = {PIN_COLSEL_0, PIN_COLSEL_1, PIN_COLSEL_2};
constexpr uint8_t PINS_ROW[] = {PIN_ROWSEL_0, PIN_ROWSEL_1, PIN_ROWSEL_2, PIN_ROWSEL_3};