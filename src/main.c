#include "Paproka_LED_Lamp.h"
#include "LED_Default_Functions.h"
#include "mqtt_broker.h"
#include "esp_wifi.h"
#include "esp_system.h"

#define LAMP_GPIO_PIN 18

#define DEFAULT_TICK_DELAY (450 / portTICK_PERIOD_MS)
TickType_t delay = 1000; // Experimental delay
#define NUM_LEDS 24

// define PUBLIC_BROKER_URI "mqtt://test.mosquitto.org:1883"
// define PUBLIC_BROKER_URI "mqtt://test.mosquitto.org:1883"
#define BROKER_URI "mqtt://10.44.25.223:1883"
#define AUTH_USR "led_lamp"
#define AUTH_PWD "MQTT_PAPROKA_LED_LAMP"

// #define WIFI_SSID "FRITZ!Box 7590 KU"
// #define WIFI_PASS "84033941904315170480"

//#define WIFI_SSID "paproka_wifi_handy"
//#define WIFI_PASS "brotchen123."

#define WIFI_SSID "EDAG-Guest"
#define WIFI_PASS "EDAG#Guest"

#define MAX_RETRIES 5

static int retry_count = 0;
static uint32_t last_attempt = 0;

static bool wifi_established = false, event_loop_initialized;

void reconnect_wifi_task(void *pvParameter)
{
    if (wifi_established)
    {
        vTaskDelete(NULL);
    }
    esp_wifi_connect();
    vTaskDelete(NULL);
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT || event_base == IP_EVENT)
    {
        switch (event_id)
        {
        case (WIFI_EVENT_STA_START): // ID 2
            wifi_established = false;
            ESP_LOGI("WIFI - ATTEMPTING CONNECTION", "connect to the AP");
            esp_wifi_connect();
            break;

        case (WIFI_EVENT_STA_CONNECTED):
            ESP_LOGI("WIFI - CONNECTED", "Wifi connected");
            break;

        case (WIFI_EVENT_STA_DISCONNECTED):
            if (wifi_established)
            {
                if (retry_count < MAX_RETRIES)
                {
                    retry_count++;
                    last_attempt = esp_log_timestamp();
                    esp_wifi_disconnect();
                    ESP_LOGI("WIFI - RETRY CONNECTION", "retrying... (%d/%d)", retry_count, MAX_RETRIES);
                    vTaskDelay(5000 / portTICK_PERIOD_MS);
                    xTaskCreate(reconnect_wifi_task, "reconnect_wifi", 4096, NULL, 1, NULL);
                }
                else
                {
                    ESP_LOGI("WIFI - GIVING UP", "Restarting ESP32");
                    esp_restart();
                }
            }
            break;
        case (WIFI_EVENT_HOME_CHANNEL_CHANGE):
            ESP_LOGI("WIFI - RETRY CONNECTION", "retry to connect to the AP");
            xTaskCreate(reconnect_wifi_task, "reconnect_wifi", 4096, NULL, 1, NULL);
            break;

        case (IP_EVENT_STA_GOT_IP):
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI("Wifi connected. IP retrieve", "got ip:" IPSTR, IP2STR(&(event->ip_info.ip)));
            wifi_established = true;
            mqtt_init(BROKER_URI, AUTH_USR, AUTH_PWD, NUM_LEDS);
            // mqtt_init(PUBLIC_BROKER_URI, AUTH_USR, AUTH_PWD, NUM_LEDS);
            break;

        default:
            ESP_LOGI("WIFI - UNKNOWN WITH BASE", "Unhandled event with ID: %li and base: %s", event_id, event_base);
            wifi_established = false;
            ESP_LOGI("WIFI - RETRY CONNECTION", "retry to connect to the AP");
            vTaskDelay(5000 / portTICK_PERIOD_MS); // 5s delay
            esp_wifi_connect();

            break;
        }
    }
    else if (event_base)
    {
        ESP_LOGI("WIFI - UNKNOWN WITH BASE, out switch", "Unhandled event with ID: %li and base: %s", event_id, event_base);
    }
    else
    {
        ESP_LOGI("WIFI - UNKNOWN WITHOUT BASE, out switch", "Unhandled event with ID: %li and a null base", event_id);
    }
}

void config_wifi()
{

    esp_netif_init(); // 1
    if (!event_loop_initialized)
    {
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        event_loop_initialized = true;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // 2
    cfg.nvs_enable = false;
    ESP_ERROR_CHECK(esp_wifi_init(&cfg)); // 3
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_LOST_IP, &wifi_event_handler, NULL)); // 4
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    esp_netif_create_default_wifi_sta();                                 // 5
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));                   // WIFI STATION MODE
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config)); // 6
    ESP_ERROR_CHECK(esp_wifi_start());                                   // 7
}

void app_main()
{
    esp_log_level_set("*", ESP_LOG_VERBOSE); // Full debug

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
    sendData();
    vTaskDelay(DEFAULT_TICK_DELAY * 2);

    config_wifi();

    while (true)
    {
        vTaskDelay(DEFAULT_TICK_DELAY * 2);
    }
}