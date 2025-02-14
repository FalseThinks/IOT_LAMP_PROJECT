/*
    Implements coloring functions.
    Used for device: WS2813-Mini
    Number of LEDs: 24
*/

#ifndef LED_DEFAULT_FUNCTIONS_H
#define LED_DEFAULT_FUNCTIONS_H

#include "freertos/FreeRTOS.h"
#include "Paproka_LED_Lamp.h"

// COMMON COLORS
#define BLACK_HEX 0x000000
#define WHITE_HEX 0xFFFFFF
#define RED_HEX 0xFF0000
#define GREEN_HEX 0x00FF00
#define BLUE_HEX 0x0000FF
#define YELLOW_HEX 0xFFFF00
#define ORANGE_HEX 0xFF8800
#define INDIGO_HEX 0x4B0082
#define VIOLET_HEX 0x7F00FF

/**
 * @brief Sets up variables.
 * @param numleds Amount of Lamp's LEDs.
 * @param tick_delay ESP32 tick delay.
 * @param b Brightness value.
 * @param leds_array Leds array.
 * @author paproka
 */
void init(uint8_t numleds, TickType_t tick_delay, uint8_t b, color_t *leds_array);

/**
 * @brief Sets Spanish Flag
 * @author paproka
 */
void spanish_flag();

/**
 * @brief Sets Andalusia Flag
 * @author paproka
 */
void andalusia_flag();

/**
 * 
 */
void italian_flag();

/**
 * @brief Sets a happy face
 * @author paproka
 */
void happy_face();

/**
 * @brief Starts coloring from start to end, and then backwards, with a certain color.
 * @param HEX HEX code to fill lamp
 */
void full_to_empty(uint32_t HEX);

/**
 * @brief Makes a single color lane  to flush around the lamp.
 * @param HEX HEX code to flush around.
 * @param count Length of row ( must be between 0 and numLeds-1 ).
 * @author paproka
 */
void flush_one(uint32_t HEX, uint8_t count);

/**
 * @brief Makes two color lanes to flush around the lamp. Leaves an space between them.
 * @param HEX1 HEX code of first lane to flush around.
 * @param HEX2 HEX code of second lane to flush around.
 * @param count Length of row ( must be between 0 and (numLeds-2)/2 ).
 * @author paproka
 */
void flush_two(uint32_t HEX1, uint32_t HEX2, uint8_t count);

/**
 * @brief Makes three color lanes to flush around the lamp. Leaves an space between them.
 * @param HEX1 HEX code of first lane to flush around.
 * @param HEX2 HEX code of second lane to flush around.
 * @param HEX3 HEX code of third lane to flush around.
 * @param count Length of row ( must be between 0 and (numLeds-3)/3 ).
 * @author paproka
 */
void flush_three(uint32_t HEX1, uint32_t HEX2, uint32_t HEX3, uint8_t count);

/**
 * @brief Sets up a rainbow in LED.
 * @author paproka
 */
void rainbow(); // HAS A BUG! BRIGHTNESS IS NOT MODIFIED.

#endif