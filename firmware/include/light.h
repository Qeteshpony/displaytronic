#pragma once

void lightInit();
void lightSetBrightness256(uint8_t brightness);
void lightSetBrightness1024(uint16_t brightness);
uint8_t lightGetBrightness256();
uint16_t lightGetBrightness1024();
