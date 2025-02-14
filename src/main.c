#include "Paproka_LED_Lamp.h"
#include "LED_Default_Functions.h"

#include "esp_system.h"

#define LAMP_GPIO_PIN 18

#define DEFAULT_TICK_DELAY (450 / portTICK_PERIOD_MS)
TickType_t delay = 1000; // Experimental delay
#define NUM_LEDS 24


void app_main()
{
    if (setup(LAMP_GPIO_PIN, NUM_LEDS, DEFAULT_TICK_DELAY) != ESP_OK)
    {
        ESP_LOGI("APP_MAIN", "Setup failed. Exiting...");
        return;
    }

    if (clearLeds() != ESP_OK)
    {
        ESP_LOGI("APP_MAIN", "Failed to clear LEDs.");
        return;
    }
    vTaskDelay(DEFAULT_TICK_DELAY*2);

    // Note: Predefined functions from LED_DEFAULT_FUNCTIONS sends data automatically.
    
    // FLAGS SECTION
    //spanish_flag();
    andalusia_flag();
    //italian_flag();
    vTaskDelay(DEFAULT_TICK_DELAY*2); 

    // HAPPY FACE SECTION
    // Note: needs a clear due to change of layout
    clearLeds();
    happy_face();
    vTaskDelay(DEFAULT_TICK_DELAY*2);

    // FULL TO EMPTY SECTION
    clearLeds();
    full_to_empty(BLUE_HEX);
    vTaskDelay(DEFAULT_TICK_DELAY*2);

    // RAINBOW SECTION
    clearLeds();
    rainbow();
    vTaskDelay(DEFAULT_TICK_DELAY*2);

    // DEFAULT FLUSHES SECTION
    clearLeds();
    
    //flush_one(RED_HEX, 4);
    //flush_two(BLUE_HEX, GREEN_HEX, 5);
    //flush_three(GREEN_HEX, WHITE_HEX, RED_HEX, 3);
    //vTaskDelay(DEFAULT_TICK_DELAY*2);

    // OTHER FUNCTIONS (DEFINE LAYOUT BEFORE)
    //setIntermitent();
    //setFlush();
    //setFlushAndIntermitent();

}