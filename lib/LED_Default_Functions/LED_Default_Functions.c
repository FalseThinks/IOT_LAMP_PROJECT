// -- LED FUNCTIONS --

#include "LED_Default_Functions.h"


static uint8_t numLeds;
static TickType_t TICK_DELAY;
static uint8_t brightness;
static color_t *leds;

void init(uint8_t numleds, TickType_t tick_delay, uint8_t b, color_t *leds_array){
    numLeds = numleds;
    TICK_DELAY = tick_delay;
    brightness = b;
    leds = leds_array;
}

void spanish_flag()
{
    clearLeds();
    fillHEX(RED_HEX, 0, 3);
    fillHEX(YELLOW_HEX, 3, 3);
    fillHEX(RED_HEX, 6, 3);

    fillHEX(RED_HEX, 12, 3);
    fillHEX(YELLOW_HEX, 15, 3);
    fillHEX(RED_HEX, 18, 3);
    sendData();
}

void andalusia_flag()
{
    clearLeds();
    fillHEX(GREEN_HEX, 0, 3);
    fillHEX(WHITE_HEX, 3, 3);
    fillHEX(GREEN_HEX, 6, 3);

    fillHEX(GREEN_HEX, 12, 3);
    fillHEX(WHITE_HEX, 15, 3);
    fillHEX(GREEN_HEX, 18, 3);
    sendData();
}

void italian_flag()
{
    clearLeds();
    fillHEX(GREEN_HEX, 0, 3);
    fillHEX(WHITE_HEX, 3, 3);
    fillHEX(RED_HEX, 6, 3);

    fillHEX(GREEN_HEX, 12, 3);
    fillHEX(WHITE_HEX, 15, 3);
    fillHEX(RED_HEX, 18, 3);
    sendData();
}

void happy_face()
{
    clearLeds();
    fillHEX(YELLOW_HEX, 0, 3);
    fillHEX(YELLOW_HEX, 6, 3);
    fillHEX(YELLOW_HEX, 13, 7);
    sendData();
}

void rainbow(){
    clearLeds();
    // red, orange, yellow, green, blue, indigo, and violet
    uint32_t colors[] = {
        RED_HEX,    ORANGE_HEX, YELLOW_HEX, GREEN_HEX, BLUE_HEX, INDIGO_HEX, VIOLET_HEX
    };

    for (uint8_t i = 0; i <= 20; i++) {
        setHEX(i, colors[i%7]);
    }
    sendData();
}

void full_to_empty(uint32_t HEX)
{
    clearLeds();
    for (int i = 0; i < numLeds; i++)
    {
        setHEX(i, HEX);
        sendData();
        vTaskDelay(TICK_DELAY / 2);
    }
    for (int i = numLeds - 1; i >= 0; i--)
    {
        setHEX(i, BLACK_HEX);
        sendData();
        vTaskDelay(TICK_DELAY / 2);
    }
}

uint8_t setRGBLane_flush(uint8_t actual, uint32_t HEX, uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        setHEX(actual, HEX); // Set the LED at position 'actual' to the color 'HEX'
        actual++; // Move to the next LED
        if (actual >= numLeds) {
            actual = 0; // Wrap around if we reach the end of the LED strip
        }
    }
    return actual;
}

void flush_one(uint32_t HEX, uint8_t count)
{
    clearLeds();

    if (count > numLeds-1 || count == 0)
    {
        ESP_LOGI("NOT_ALLOWED", "Count value must be different, for taking effect");
        return;
    }

    uint8_t first = 0;

    while (true)
    {

        fillHEX(BLACK_HEX, 0, 24);

        uint8_t actual = first;
        actual = setRGBLane_flush(actual,HEX,count);

        sendData();
        vTaskDelay(TICK_DELAY / 4);
        first++;
        if (first >= numLeds)
        {
            first = 0;
        }
    }
}

void flush_two(uint32_t HEX1, uint32_t HEX2, uint8_t count)
{
    clearLeds();
    uint8_t first = 0;

    if (count > (numLeds-2)/2 || count==0)
    {
        ESP_LOGI("NOT_ALLOWED", "Count value must be fewer that 16, for taking effect");
        return;
    }

    while (true)
    {
        fillHEX(BLACK_HEX, 0, 24);
        
        uint8_t actual = first;
        actual = setRGBLane_flush(actual,HEX1,count);

        //RESET VALUES
        actual++;
        if(actual>=numLeds)
            actual=0;
        
        actual = setRGBLane_flush(actual,HEX2,count);

        sendData();
        vTaskDelay(TICK_DELAY / 4);
        first++;
        if (first >= numLeds)
        {
            first = 0;
        }
    }
}

void flush_three(uint32_t HEX1, uint32_t HEX2, uint32_t HEX3, uint8_t count){
    clearLeds();
    uint8_t first = 0;

    if (count > (numLeds-3)/3  || count==0)
    {
        ESP_LOGI("NOT_ALLOWED", "Count value must be fewer that 16, for taking effect");
        return;
    }

    while (true)
    {
        fillHEX(BLACK_HEX, 0, 24);

        uint8_t actual = first;
        actual = setRGBLane_flush(actual,HEX1,count);

        //RESET VALUES
        actual++;
        if(actual>=numLeds)
            actual=0;

        actual = setRGBLane_flush(actual,HEX2,count);

        //RESET VALUES
        actual++;
        if(actual>=numLeds)
            actual=0;

        actual = setRGBLane_flush(actual,HEX3,count);

        sendData();
        vTaskDelay(TICK_DELAY / 4);
        first++;
        if (first >= numLeds)
        {
            first = 0;
        }
    }
}