#include "Paproka_LED_Lamp.h"
#include "driver/rmt.h"

#define ESP32_RMT_CHANNEL_MAX RMT_CHANNEL_MAX

#define RMT_LL_HW_BASE  (&RMT)

static uint32_t t0h_ticks = 0; // We have to check manually for better management.
static uint32_t t1h_ticks = 0;
static uint32_t t0l_ticks = 0;
static uint32_t t1l_ticks = 0;

bool rmt_reserved_channels[ESP32_RMT_CHANNEL_MAX];

static void IRAM_ATTR ws2812_rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size,
        size_t wanted_num, size_t *translated_size, size_t *item_num)
{
    if (src == NULL || dest == NULL) {
        *translated_size = 0;
        *item_num = 0;
        return;
    }
    const rmt_item32_t bit0 = {{{ t0h_ticks, 1, t0l_ticks, 0 }}}; //Logical 0
    const rmt_item32_t bit1 = {{{ t1h_ticks, 1, t1l_ticks, 0 }}}; //Logical 1
    size_t size = 0;
    size_t num = 0;
    uint8_t *psrc = (uint8_t *)src;
    rmt_item32_t *pdest = dest;
    while (size < src_size && num < wanted_num) {
        for (int i = 0; i < 8; i++) {
            // MSB first
            if (*psrc & (1 << (7 - i))) {
                pdest->val =  bit1.val;
            } else {
                pdest->val =  bit0.val;
            }
            num++;
            pdest++;
        }
        size++;
        psrc++;
    }
    *translated_size = size;
    *item_num = num;
}

void espShow_old(uint8_t pin, uint8_t *leds, uint32_t numBytes) {
    // Reserve channel
    rmt_channel_t channel = ESP32_RMT_CHANNEL_MAX;
    for (size_t i = 0; i < ESP32_RMT_CHANNEL_MAX; i++) {
        if (!rmt_reserved_channels[i]) {
            rmt_reserved_channels[i] = true;
            channel = i;
            break;
        }
    }
    if (channel == ESP32_RMT_CHANNEL_MAX) {
        // Ran out of channels!
        return;
    }


    // Match default TX config from ESP-IDF version 3.4
    rmt_config_t config = {
        .rmt_mode = RMT_MODE_TX,
        .channel = channel,
        .gpio_num = pin,
        .clk_div = 2,
        .mem_block_num = 1,
        .tx_config = {
            .carrier_freq_hz = 38000,
            .carrier_level = RMT_CARRIER_LEVEL_HIGH,
            .idle_level = RMT_IDLE_LEVEL_LOW,
            .carrier_duty_percent = 33,
            .carrier_en = false,
            .loop_en = false,
            .idle_output_en = true,
        }
    };
    rmt_config(&config);
    rmt_driver_install(config.channel, 0, 0);

    // Convert NS timings to ticks
    uint32_t counter_clk_hz = 0;


    rmt_get_counter_clock(channel, &counter_clk_hz);

    // NS to tick converter
    float ratio = (float)counter_clk_hz / 1e9;

    t0h_ticks = (uint32_t)(ratio * WS2813_MINI_T0H_NS);
    t0l_ticks = (uint32_t)(ratio * WS2813_MINI_T0L_NS);
    t1h_ticks = (uint32_t)(ratio * WS2813_MINI_T1H_NS);
    t1l_ticks = (uint32_t)(ratio * WS2813_MINI_T1L_NS);

    // Initialize automatic timing translator
    rmt_translator_init(config.channel, ws2812_rmt_adapter);

    // Write and wait to finish
    rmt_write_sample(config.channel, leds, (size_t)numBytes, true);
    rmt_wait_tx_done(config.channel, pdMS_TO_TICKS(100));

    // Free channel again
    rmt_driver_uninstall(config.channel);
    rmt_reserved_channels[channel] = false;

    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
}