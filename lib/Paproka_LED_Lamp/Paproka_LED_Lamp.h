/*
Based on Adafruit_Neopixel library.
https://github.com/adafruit/Adafruit_NeoPixel

Used for device: WS2813-Mini
Number of LEDs: 24
*/

#ifndef PAPROKA_LED_LAMP_H
#define PAPROKA_LED_LAMP_H

#include "esp_log.h"
#include "esp_err.h"

#include <stdlib.h>
#include <stdint.h>
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp32/rom/ets_sys.h"

#include "esp_rmt_old.h"
#include <string.h>

// --STATUS MESSAGES START--

#define SETUP_OK "SETUP_OK"
#define CLEAR_LEDS_OK "CLEAR_LEDS_OK"
#define RESET_LEDS_OK "RESET_LEDS_OK"
#define SET_RGB_OK "SET_RGB_OK"
#define SET_HEX_OK "SET_HEX_OK"
#define SET_COLOR_OK "SET_COLOR_OK"
#define FILL_RGB_OK "FILL_RGB_OK"
#define FILL_HEX_OK "FILL_HEX_OK"
#define SEND_DATA_OK "SEND_DATA_OK"
#define GPIO_SET_OK "GPIO_SET_OK"
#define RMT_OK "RMT_OK"
#define MUTEX_OK "MUTEX_OK"

#define SETUP_FAIL "SETUP_FAIL"
#define CLEAR_LEDS_FAIL "CLEAR_LEDS_FAIL"
#define RESET_LEDS_FAIL "RESET_LEDS_FAIL"
#define SET_RGB_FAIL "SET_RGB_FAIL"
#define SET_HEX_FAIL "SET_HEX_FAIL"
#define SET_COLOR_FAIL "SET_COLOR_FAIL"
#define FILL_RGB_FAIL "FILL_RGB_FAIL"
#define FILL_HEX_FAIL "FILL_HEX_FAIL"
#define SEND_DATA_FAIL "SEND_DATA_FAIL"
#define GPIO_SET_FAIL "GPIO_SET_FAIL"
#define RMT_FAIL "RMT_FAIL"
#define MUTEX_FAIL "MUTEX_FAIL"

#define RMT "RMT"

// --STATUS MESSAGES END --

#define WS2813_MINI_RES_DELAY 350 // 350µs

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

/**
 * @brief Sets up LEDs Lamp in desired pin.
 * @param pin Desired pin to be configured.
 * @param numleds Amount of LEDs that the lamp has got.
 * @param tick_delay Amount of ticks that ESP32 handle.
 * @note WS2813-Mini always uses GRB, but format is given in RGB, so you do not need to worry about it.
 * @return ESP_OK if everything is correct. Else would return ESP_FAIL.
 * @author paproka
 */
esp_err_t setup(gpio_num_t pin, uint8_t numleds, TickType_t tick_delay);

/**
 * @brief Clear all leds and turn them to 0.
 * @return ESP_OK if everything is correct. Else would return ESP_FAIL.
 * @author paproka
 */
esp_err_t clearLeds();

/**
 * @brief Set RGB color in a certain led.
 * @param led_number Led number in the wire.
 * @param r Red value (0-255).
 * @param g Green value (0-255).
 * @param b Blue value (0-255).
 * @note It transforms it to color_t struct.
 * @return ESP_OK if everything is correct. Else would return ESP_FAIL.
 * @author paproka
 */
esp_err_t setRGB(uint8_t led_number, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Set HEX color in a certain led.
 * @param led_number Led number in the wire.
 * @param HEX HEX color value.
 * @note It transforms it to color_t struct.
 * @return ESP_OK if everything is correct. Else would return ESP_FAIL.
 * @author paproka
 */
esp_err_t setHEX(uint8_t led_number, uint32_t HEX);

/**
 * @brief Set color_t type in a certain led. Used by setRGB and setHEX.
 * @param led_number Led number in the wire.
 * @param RGB color_t data.
 * @return ESP_OK if everything is correct. Else would return ESP_FAIL.
 * @author paproka
 */
esp_err_t setColor(uint8_t led_number, color_t RGB);

/**
 * @brief Transform an RGB format color to HEX value.
 * @param r Red value (0-255).
 * @param g Green value (0-255).
 * @param b Blue value (0-255).
 * @return HEX value of given RGB.
 */
uint32_t RGBtoHEX(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Transform HEX value to RGB format, saved in struct.
 * @param hex Color in HEX code.
 * @return Color struct, which separes in RGB.
 * @note Check color_t struct.
 * @return color_t struct of given HEX.
 * @author paproka
 */
color_t HEXtoRGB(uint32_t HEX);

/**
 * @brief Fills lamp with a certain color, in RGB data. You can specify first pixel start and a count of pixels.
 * @param r Red value (0-255).
 * @param g Green value (0-255).
 * @param b Blue value (0-255).
 * @param first First led to start fillment (0 from start).
 * @param count The amount of leds to be filled from first.
 * @return ESP_OK if everything is correct. Else would return ESP_FAIL.
 * @author paproka
 */
esp_err_t fillRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t first, uint8_t count);

/**
 * @brief Fills lamp with a certain color, in HEX data. You can specify first pixel start and a count of pixels.
 * @param HEX Hex color value. Default value is WHITE.
 * @param first First led to start fillment (0 from start).
 * @param count The amount of leds to be filled from first.
 * @return ESP_OK if everything is correct. Else would return ESP_FAIL.
 * @author paproka
 */
esp_err_t fillHEX(uint32_t HEX, uint8_t first, uint8_t count);

/**
 * @brief Sets lamp brightness. Uses HSL.
 * @param b Brightness value (0-255).
 * @author paproka
 */
void setBrightness(uint8_t b);

/**
 * @brief Sets an intermitent in the lamp. (Bright goes from 1 to 255).
 * @author paproka
 */
void setIntermitent();

/**
 * @brief Sets a flush in the lamp. (Makes colors to move around the lamp).
 * @author paproka
 */
void setFlush();

/**
 * @brief Sets a flush with intermitent in the lamp.
 * @note Mixes setIntermitent() and setFlush().
 * @author paproka
 */
void setFlushAndIntermitent();

/**
 * @brief Sends current updates to lamp.
 * @note Has to be called after updates done to lamp.
 * @return ESP_OK if everything is correct. Else would return ESP_FAIL.
 * @author paproka
 */
esp_err_t sendData();

// HSL
typedef struct
{
    float h; // HUE
    float s; // SATURATION
    float l; // LIGHTNESS
} hsl_t;

/**
 * @brief Converts RGB value to its equivalent in HSL.
 * Based on a solution provided in Github, adapted to C.
 * @ref https://gist.github.com/vahidk/05184faf3d92a0aa1b46aeaa93b07786
 * @param r Red value (0-255).
 * @param g Green value (0-255).
 * @param b Blue value (0-255).
 * @return HSL structure of given RGB.
 * @author paproka
 */
hsl_t RGBtoHSL(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Converts HSL value to its equivalent in RGB
 * Based on a solution provided in Github, adapted to C.
 * @ref https://gist.github.com/vahidk/05184faf3d92a0aa1b46aeaa93b07786
 * @param HSL HSL structure.
 * @return RGB structure of given HSL.
 * @note See structure definition.
 * @author paproka
 */
color_t HSLtoRGB(hsl_t HSL);

/**
 * @brief Sets single brightness attribute for a led. 
 * Must specify original color, and then bright will be changed.
 * @param r Red value (0-255).
 * @param g Green value (0-255).
 * @param b Blue value (0-255).
 * @param bright Bright value (0-255).
 * @return Color_t structure of given modification.
 * @author paproka
 */
color_t setBrightnessSingleRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t bright);

/**
 * @brief Get actual led color.
 * @param index Led number.
 * @return Color of specified LED.
 * @author paproka
 */
color_t getLedColor(int index);

#endif