#include "Paproka_LED_Lamp.h"
#include "LED_Default_Functions.h"

#include "esp_timer.h"
#include <math.h>

static color_t *leds;
static uint8_t brightness;
static uint8_t numLeds;
static gpio_num_t GPIO_PIN;
static TickType_t TICK_DELAY;

void convertColorsToBytes(color_t *leds, uint8_t *ledBytes, size_t numLEDs)
{
    for (uint8_t i = 0; i < numLEDs; i++)
    {
        // Convert RGB to HEX (uint32_t)
        // ADAPTING GRB-A-LIKE SENDING
        uint32_t hexColor = RGBtoHEX(leds[i].g, leds[i].r, leds[i].b);

        // Extract the bytes (R, G, B) from the 32-bit integer
        ledBytes[i * 3] = (hexColor >> 16) & 0xFF;    // Green component
        ledBytes[i * 3 + 1] = (hexColor >> 8) & 0xFF; // Red component
        ledBytes[i * 3 + 2] = hexColor & 0xFF;        // Blue component
    }
}

esp_err_t sendData()
{
    if (leds == NULL || numLeds == 0 || GPIO_PIN < 0 || GPIO_PIN >= GPIO_NUM_MAX)
    {
        ESP_LOGI(SEND_DATA_FAIL, "Data has failed to be sent.");
        return ESP_FAIL;
    }

    uint8_t *ledBytes = (uint8_t *)malloc(numLeds * 3);
    convertColorsToBytes(leds, ledBytes, numLeds);

    espShow_old(GPIO_PIN, ledBytes, numLeds * 3);

    return ESP_OK;
}

esp_err_t setup(gpio_num_t pin, uint8_t numleds, TickType_t tick_delay)
{
    if (numleds > 0 && pin >= 0)
    {
        numLeds = numleds;
        GPIO_PIN = pin;
        TICK_DELAY = (tick_delay == 0) ? (1000 / portTICK_PERIOD_MS) : tick_delay;

        gpio_reset_pin(GPIO_PIN);
        gpio_set_direction(GPIO_PIN, GPIO_MODE_OUTPUT);

        // As format is GRB, we need to care about that!!

        leds = (color_t *)malloc(numLeds * sizeof(color_t));

        if (leds != NULL)
        {
            memset(leds, 0, numLeds * sizeof(color_t));
            setBrightness(20); // Initialize brightness at low level
            init(numleds, TICK_DELAY, brightness, leds);
            sendData();
            ESP_LOGI(SETUP_OK, "Setup is CORRECT.");
            return ESP_OK;
        }
        else
        {
            ESP_LOGI(SETUP_FAIL, "Setup is INCORRECT. Failed to allocate memory.");
            return ESP_FAIL;
        }
    }
    else
    {
        ESP_LOGI(SETUP_FAIL, "Setup is INCORRECT.");
        return ESP_FAIL;
    }
}

esp_err_t clearLeds()
{
    if (leds != NULL && numLeds != 0)
    {
        for (uint8_t i = 0; i < numLeds; i++)
        {
            setRGB(i, 0, 0, 0);
        }

        ESP_LOGI(CLEAR_LEDS_OK, "Leds where successfully cleared.");
        return ESP_OK;
    }
    ESP_LOGI(CLEAR_LEDS_FAIL, "Fail has ocurred in clearing leds.");
    return ESP_FAIL;
}

uint32_t RGBtoHEX(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 16) | (g << 8) | b; // 8 for each
}

color_t HEXtoRGB(uint32_t HEX)
{
    color_t color;
    color.r = (HEX >> 16) & 0xFF;
    color.g = (HEX >> 8) & 0xFF;
    color.b = HEX & 0xFF;
    return color;
}

esp_err_t setColor(uint8_t led_number, color_t RGB)
{
    if (leds != NULL)
    {
        leds[led_number] = RGB;
        // ESP_LOGI(SET_COLOR_OK, "Color was succesfully set.");
        return ESP_OK;
    }

    ESP_LOGI(SET_COLOR_FAIL, "Color was not set.");
    return ESP_FAIL;
}

esp_err_t setRGB(uint8_t led_number, uint8_t r, uint8_t g, uint8_t b)
{
    if (led_number >= numLeds)
    {
        ESP_LOGI(SET_RGB_FAIL, "Expected led is incorrect.");
        return ESP_FAIL;
    }
    if (brightness) // Has to check if brightness is set, for multiplying each value.
    {
        r = (r * brightness) >> 8; // Right-shifts 8 as a reescaling value.
        g = (g * brightness) >> 8;
        b = (b * brightness) >> 8;
    }

    // GRB!
    uint32_t HEX = RGBtoHEX(r, g, b);
    // printf("HEX_VALUE = %li", HEX);
    return setColor(led_number, HEXtoRGB(HEX));
}

esp_err_t setHEX(uint8_t led_number, uint32_t HEX)
{
    if (led_number >= numLeds)
    {
        ESP_LOGI(SET_HEX_FAIL, "Expected led is incorrect.");
        return ESP_FAIL;
    }
    color_t RGB = HEXtoRGB(HEX);
    return setColor(led_number, RGB);
}

esp_err_t fillRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t first, uint8_t count)
{
    if (first >= numLeds)
        first = 0; // Reset when last led is lightened.
    if (first + count > numLeds)
    {
        ESP_LOGI(FILL_RGB_FAIL, "Expected data is incorrect. Check desired and count led.");
        return ESP_FAIL;
    }
    for (uint8_t i = 0; i < count; i++)
    {
        if (setRGB(first + i, r, g, b) == ESP_FAIL)
            return ESP_FAIL;
    }
    ESP_LOGI(FILL_RGB_OK, "Filled succesfully.");
    return ESP_OK;
}

esp_err_t fillHEX(uint32_t HEX, uint8_t first, uint8_t count)
{
    color_t RGB = HEXtoRGB(HEX);

    return fillRGB(RGB.r, RGB.g, RGB.b, first, count);
}

/*void setBrightness(uint8_t b)
{
    uint8_t b_fix = b + 1; //Avois shifting left in 0 value
    if (b_fix != brightness)
    {
        uint8_t oldBrightness = brightness-1;
        uint16_t scale; // Multiplies each pixel for the scale factor.

        if (oldBrightness == 0)
            scale = 0;
        else if (b == 255)
            scale = 65535 / oldBrightness;
        else
            scale = (((uint16_t)b_fix << 8) - 1) / oldBrightness; // Shifting 8 left is the same as multiply x256
        for (uint16_t i = 0; i < numLeds; i++)
        {
            // Readjust each color value, given scale.
            color_t RGB = leds[i];
            RGB.b = (RGB.b * scale) >> 8;
            RGB.g = (RGB.g * scale) >> 8;
            RGB.r = (RGB.r * scale) >> 8;
            leds[i] = RGB;
        }
        brightness = b_fix;
    }
}*/
// This solution is provided by Adafruit. I am going to do my own, based on Hue.

hsl_t RGBtoHSL(uint8_t r, uint8_t g, uint8_t b)
{
    float rf = r / 255.0f, gf = g / 255.0f, bf = b / 255.0f;
    float max = (rf > gf) ? ((rf > bf) ? rf : bf) : ((gf > bf) ? gf : bf);
    float min = (rf < gf) ? ((rf < bf) ? rf : bf) : ((gf < bf) ? gf : bf);

    float h, s, l = (max + min) / 2.0f;
    if (max == min)
        h = s = 0; // grayscale (no saturation)
    else
    {
        float d = max - min;
        s = (l > 0.5f) ? d / (2.0f - max - min) : d / (max - min);
        if (max == rf)
            h = (gf - bf) / d + (gf < bf ? 6.0f : 0.0f);
        else if (max == gf)
            h = (bf - rf) / d + 2.0f;
        else
            h = (rf - gf) / d + 4.0f;
    }
    h *= 60.0f;

    hsl_t HSL = {h, s*100.0f, l*100.0f};
    return HSL;
}

color_t HSLtoRGB(hsl_t HSL)
{
    float h = HSL.h;
    float s = HSL.s / 100.0f;
    float l = HSL.l / 100.0f;

    float c = (1.0f - fabsf(2.0f * l - 1.0f)) * s;
    float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
    float m = l - c / 2.0f;

    float rf = 0, gf = 0, bf = 0;

    if (h < 60) {
        rf = c, gf = x, bf = 0;
    } else if (h < 120) {
        rf = x, gf = c, bf = 0;
    } else if (h < 180) {
        rf = 0, gf = c, bf = x;
    } else if (h < 240) {
        rf = 0, gf = x, bf = c;
    } else if (h < 300) {
        rf = x, gf = 0, bf = c;
    } else {
        rf = c, gf = 0, bf = x;
    }

    color_t RGB;
    RGB.r = (uint8_t) roundf((rf + m) * 255.0f);
    RGB.g = (uint8_t) roundf((gf + m) * 255.0f);
    RGB.b = (uint8_t) roundf((bf + m) * 255.0f);

    return RGB;
}

color_t setBrightnessSingleRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t bright){
    //Black is black
    if (r == 0 && g == 0 && b == 0){
        return (color_t){0,0,0};
    }
    static uint8_t original_brightness = 255; // Store max brightness by default

    hsl_t HSL = RGBtoHSL(r, g, b);

    // If this is the first brightness adjustment, store the original value
    if (brightness == 255) {
        original_brightness = HSL.l;
    }

    // Scale using the originally stored brightness, not the modified one
    HSL.l = (original_brightness * bright) / 255.0f;

    return HSLtoRGB(HSL);
}

void setBrightness(uint8_t b)
{
    if (b!=brightness)
    {
        for (uint8_t i=0; i<numLeds; i++)
        {
            color_t RGB = leds[i];
            leds[i] = setBrightnessSingleRGB(RGB.r, RGB.g, RGB.b, b);
        }
        brightness = b;
    }
}

void setIntermitent(){
    uint8_t value1 = 1;
    uint8_t value2 = 255;
    while(true){
        setBrightness(value1);
        sendData();

        vTaskDelay(TICK_DELAY*2);

        setBrightness(value2);
        sendData();

        vTaskDelay(TICK_DELAY*2);
    }
}

void setFlush(){
    color_t *leds_aux = (color_t *)malloc(numLeds * sizeof(color_t)); 
    if (leds_aux!=NULL){
        while(true){
            memcpy(leds_aux,leds, sizeof(color_t)*numLeds);
            leds[0] = leds_aux[numLeds-1];
            for (int i=0; i<numLeds-1; i++){
                leds[i+1] = leds_aux[i];
            }
            sendData();
            vTaskDelay(TICK_DELAY/2);
        }
    }
    return; 
}

void setFlushAndIntermitent(){
    uint8_t value1 = 1;
    uint8_t value2 = 255;
    color_t *leds_aux = (color_t *)malloc(numLeds * sizeof(color_t));
    int i=0; 
    if (leds_aux!=NULL){
        while(true){
            uint8_t bright_value = (i%2==0)?value1:value2;
            memcpy(leds_aux,leds, sizeof(color_t)*numLeds);
            leds[0] = leds_aux[numLeds-1];
            for (int i=0; i<numLeds-1; i++){
                leds[i+1] = leds_aux[i];
            }
            setBrightness(bright_value);
            sendData();
            i++;
            vTaskDelay(TICK_DELAY/2);
        }
    }
    return; 
}

color_t getLedColor(int index){
    return leds[index];
}