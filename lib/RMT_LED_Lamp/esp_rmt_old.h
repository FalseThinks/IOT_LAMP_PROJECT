/*
Based on Adafruit_Neopixel library - RMT Protocol.
https://github.com/adafruit/Adafruit_NeoPixel

Used for device: WS2813-Mini
Number of LEDs: 24
*/

#ifndef RMT_LED_LAMP_H
#define RMT_LED_LAMP_H

#include "driver/rmt_tx.h"
#include "driver/rmt_types.h"
#include "driver/rmt_encoder.h"

#define WS2813_MINI_T0H_NS 300 // 300ns (220-380ns)
#define WS2813_MINI_T0L_NS 800 // 800ns (580-1600ns)
#define WS2813_MINI_T1H_NS 900 // 900ns (580-1600ns)
#define WS2813_MINI_T1L_NS 300 // 300ns (220-420ns)

//void espShow(uint8_t pin, uint8_t *leds, uint32_t numBytes); -> Has to be implented in non-experimental phase

//EXPERIMENTAL. FIX PREVIOUS ONE PLEASE
void espShow_old(uint8_t pin, uint8_t *leds, uint32_t numBytes);

#endif