/*
  Based on Adafruit Neopixel ESP implementation.
  Updated on my own to match current versions, and device.

  https://github.com/adafruit/Adafruit_NeoPixel

    Used for device: WS2813-Mini
    Number of LEDs: 24
 */

#include "Paproka_LED_Lamp.h"

// LED MUTEX
static SemaphoreHandle_t show_mutex;

#define RMT_LL_HW_BASE (&RMT)

#define SEMAPHORE_TIMEOUT_MS 50

static uint32_t t0h_ticks = 0; // We have to check manually for better management.
static uint32_t t1h_ticks = 0;
static uint32_t t0l_ticks = 0;
static uint32_t t1l_ticks = 0;

// Limit the number of RMT channels available for the leds, 8 on ESP32.
// Array to store RMT channel handles

// Custom encoder structure
typedef struct
{
    rmt_encoder_t base;     // Base encoder structure
    rmt_symbol_word_t bit0; // Symbol for logical 0
    rmt_symbol_word_t bit1; // Symbol for logical 1
    size_t encoded_size;    // Total encoded size
    size_t encoded_num;     // Total encoded items
} ws2813_mini_encoder_t;

static rmt_channel_handle_t channel = NULL;
static uint32_t counter_clock_hz = 0;

static size_t ws2813_mini_encode(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state)
{
    ws2813_mini_encoder_t *ws2813_encoder = __containerof(encoder, ws2813_mini_encoder_t, base);
    rmt_symbol_word_t *dest = (rmt_symbol_word_t *)primary_data;
    const uint8_t *src = (const uint8_t *)primary_data;
    size_t translated_size = 0;
    size_t item_num = 0;

    if (src == NULL)
    {
        *ret_state = RMT_ENCODING_COMPLETE;
        return 0;
    }

    while (translated_size < data_size && item_num < ws2813_encoder->encoded_num)
    {
        for (int i = 0; i < 8; i++)
        {
            // MSB first
            if (*src & (1 << (7 - i)))
            {
                *dest++ = ws2813_encoder->bit1;
            }
            else
            {
                *dest++ = ws2813_encoder->bit0;
            }
            item_num++;
        }
        translated_size++;
        src++;
    }

    ws2813_encoder->encoded_size = translated_size;
    ws2813_encoder->encoded_num = item_num;

    *ret_state = (translated_size < data_size) ? RMT_ENCODING_MEM_FULL : RMT_ENCODING_COMPLETE;
    return translated_size;
}

static esp_err_t ws2813_mini_reset(rmt_encoder_t *encoder)
{
    ws2813_mini_encoder_t *ws2813_encoder = __containerof(encoder, ws2813_mini_encoder_t, base);
    ws2813_encoder->encoded_size = 0;
    ws2813_encoder->encoded_num = 0;
    return ESP_OK;
}

static esp_err_t ws2813_mini_del(rmt_encoder_t *encoder)
{
    ws2813_mini_encoder_t *ws2813_encoder = __containerof(encoder, ws2813_mini_encoder_t, base);
    free(ws2813_encoder);
    return ESP_OK;
}

static esp_err_t rmt_new_ws2813_mini_encoder(rmt_encoder_handle_t *ret_encoder)
{
    ws2813_mini_encoder_t *ws2813_encoder = calloc(1, sizeof(ws2813_mini_encoder_t));
    if (!ws2813_encoder)
    {
        return ESP_ERR_NO_MEM;
    }

    ws2813_encoder->base.encode = ws2813_mini_encode;
    ws2813_encoder->base.reset = ws2813_mini_reset;
    ws2813_encoder->base.del = ws2813_mini_del;

    // Initialize the bit symbols
    ws2813_encoder->bit0.duration0 = t0h_ticks; // Duration of high level for logical 0
    ws2813_encoder->bit0.level0 = 1;            // Level for the first part (high)
    ws2813_encoder->bit0.duration1 = t0l_ticks; // Duration of low level for logical 0
    ws2813_encoder->bit0.level1 = 0;            // Level for the second part (low)

    ws2813_encoder->bit1.duration0 = t1h_ticks; // Duration of high level for logical 1
    ws2813_encoder->bit1.level0 = 1;            // Level for the first part (high)
    ws2813_encoder->bit1.duration1 = t1l_ticks; // Duration of low level for logical 1
    ws2813_encoder->bit1.level1 = 0;            // Level for the second part (low)

    *ret_encoder = &ws2813_encoder->base;
    return ESP_OK;
}

void espShow(uint8_t pin, uint8_t *leds, uint32_t numBytes)
{
    if (!show_mutex)
    {
        show_mutex = xSemaphoreCreateMutex();
    }

    // ESP_LOGI(RMT, "Initializing channel...");

    if (channel == NULL) // Create the channel only once
    {
        rmt_tx_channel_config_t tx_chan_config = {
            .gpio_num = pin,
            .clk_src = RMT_CLK_SRC_DEFAULT,
            .resolution_hz = 10 * 1000 * 1000, // 10 MHz
            .mem_block_symbols = 64,
            .trans_queue_depth = 4,
        };
        if (rmt_new_tx_channel(&tx_chan_config, &channel) != ESP_OK)
        {
            ESP_LOGI(RMT_FAIL, "Failed to create RMT channel");
            return;
        }
        if (rmt_enable(channel) != ESP_OK)
        {
            ESP_LOGI(RMT_FAIL, "Failed to create RMT channel");
            return;
        }
        counter_clock_hz = tx_chan_config.resolution_hz;
    }

    // ESP_LOGI(RMT, "Starting transmission...");

    // Ensure the channel is valid before using it
    if (channel == NULL)
    {
        ESP_LOGI(RMT_FAIL, "Channel status is invalid, aborting!");
        return;
    }

    // ESP_LOGI(RMT_OK, "RMT Channel succesfully created");

    // NS to tick converter
    float ratio = (float)counter_clock_hz / 1e9;

    t0h_ticks = (uint32_t)(ratio * WS2813_MINI_T0H_NS);
    t0l_ticks = (uint32_t)(ratio * WS2813_MINI_T0L_NS);
    t1h_ticks = (uint32_t)(ratio * WS2813_MINI_T1H_NS);
    t1l_ticks = (uint32_t)(ratio * WS2813_MINI_T1L_NS);

    // Create transmit config
    rmt_transmit_config_t rmt_default_config = {
        .loop_count = 0, // No looping
        .flags = {
            .eot_level = 0, // End of transmission level
        }};

    // Create the encoder
    rmt_encoder_handle_t ws2813_mini_encoder;
    ESP_ERROR_CHECK(rmt_new_ws2813_mini_encoder(&ws2813_mini_encoder));

    // Write and wait to finish
    if (xSemaphoreTake(show_mutex, portMAX_DELAY) == pdTRUE){
        //ESP_LOGI("RMT", "Starting transmission...");

        if (rmt_transmit(channel, ws2813_mini_encoder, leds, numBytes, &rmt_default_config) != ESP_OK)
        {
            ESP_LOGI(RMT_FAIL, "rmt_transmit failed!");
            return;
        }

        //ESP_LOGI("RMT", "Transmission started!");

        if (rmt_tx_wait_all_done(channel, pdMS_TO_TICKS(5000)) != ESP_OK)
        {
            ESP_LOGI(RMT_FAIL, "RMT transmission timeout!");
            return;
        }

        xSemaphoreGive(show_mutex);

        // Free channel and encoder
        ESP_ERROR_CHECK(rmt_disable(channel));
        ESP_ERROR_CHECK(rmt_del_channel(channel));
        ESP_ERROR_CHECK(rmt_del_encoder(ws2813_mini_encoder));

        ets_delay_us(WS2813_MINI_RES_DELAY);
        gpio_set_level(pin, 0);
    }else{
        ESP_LOGI(MUTEX_FAIL, "Mutex failed to block proccess.");
        return;
    }
    
    

    
}