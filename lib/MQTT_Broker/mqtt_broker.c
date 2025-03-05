#include "mqtt_broker.h"
#include "inttypes.h"

static const char *TAG = "MQTT_CLIENT";

#define DEFAULT_STACKSIZE 4096                         // Stack size needed for MQTT tasks
#define DEFAULT_PRIORITY tskIDLE_PRIORITY              // Lowest possible task priority
#define DEFAULT_TICK_DELAY (1000 / portTICK_PERIOD_MS) // Time for delay
static TaskHandle_t xHandle = NULL;                    // Task handle

static bool mqtt_client_already_started = false;
static int num_leds;

static led_status_json_t *leds_json;

static bool isIntermitent = false;
static bool isFlush = false;

static int global_brightness = -1;

void clearLeds_broker()
{
    clearLeds();
    for (uint8_t i = 0; i < num_leds; i++)
    {
        color_t RGB = getLedColor(i);
        leds_json[i].hex_color = RGBtoHEX(RGB.r, RGB.g, RGB.b);   
    }
}

void init_led_json()
{
    leds_json = malloc(sizeof(led_status_json_t) * num_leds);
    for (int i = 0; i < num_leds; i++)
    {
        leds_json[i].hex_color = -1;   // Default not set
        leds_json[i].brightness = 255; // Default to full brightness
    }
}

void get_data_json(const char *json_string)
{
    cJSON *root = cJSON_Parse(json_string);
    if (!root)
        return;

    cJSON *led_numbers = cJSON_GetObjectItem(root, "leds");
    if (led_numbers)
    {
        for (int i = 0; i < num_leds; i++)
        {
            char index_str[12];
            snprintf(index_str, sizeof(index_str), "%d", i);

            cJSON *led = cJSON_GetObjectItem(led_numbers, index_str);
            if (led)
            {
                // Update color
                cJSON *color = cJSON_GetObjectItem(led, "c");
                if (color && cJSON_IsString(color) && color->valuestring[0] == '#')
                {
                    leds_json[i].hex_color = (uint32_t)strtol(color->valuestring + 1, NULL, 16);
                }

                // Update brightness
                cJSON *brightness = cJSON_GetObjectItem(led, "b");
                if (brightness && cJSON_IsString(brightness))
                {
                    leds_json[i].brightness = (uint8_t)atoi(brightness->valuestring);
                }
            }
        }
    }
    cJSON *bool_is_flush = cJSON_GetObjectItem(root, "is_flush");
    isFlush = cJSON_IsTrue(bool_is_flush);
    cJSON *bool_is_intermitent = cJSON_GetObjectItem(root, "is_intermitent");
    isIntermitent = cJSON_IsTrue(bool_is_intermitent);
    cJSON *global_brightness_value = cJSON_GetObjectItem(root, "global_b");
    global_brightness = (cJSON_IsString(global_brightness_value)) ? atoi(global_brightness_value->valuestring) : -1;

    cJSON_Delete(root);
}

void restartTask(void *pvParameters)
{
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)pvParameters;
    int retry_count = 0;
    const int max_retries = 5; // Number of retries before giving up

    esp_mqtt_client_stop(client);

    while (retry_count < max_retries)
    {
        vTaskDelay(DEFAULT_TICK_DELAY); // Wait before retrying

        ESP_LOGI(TAG, "Restarting MQTT client (Attempt %d/%d)...", retry_count + 1, max_retries);
        esp_err_t err = esp_mqtt_client_start(client);

        if (err == ESP_OK)
        {
            ESP_LOGI(TAG, "MQTT client restarted successfully!");
            mqtt_client_already_started = true;
            break; // Exit retry loop
        }
        else
        {
            ESP_LOGE(TAG, "MQTT restart failed, retrying...");
            retry_count++;
        }
    }

    if (retry_count == max_retries)
    {
        ESP_LOGE(TAG, "MQTT restart failed after %d attempts. Giving up.", max_retries);
        esp_restart();
    }

    vTaskDelete(NULL);
}

void loopTask(void *pvParameters)
{
    ESP_LOGI(TAG,"PERFORMING LOOP TASK");
    if (isIntermitent){
        setIntermitent();
    }else if(isFlush){
        setFlush();
    }else if (isIntermitent && isFlush){
        setFlushAndIntermitent();
    }
}

void createNewTask(const char *const TASK_NAME, esp_mqtt_client_handle_t param, TaskHandle_t *xHandle, task_type_t taskType)
{
    // Initialization
    BaseType_t xReturned = pdFALSE;

    if (*xHandle != NULL)
    {
        printf("Task already exists. Deleting it...\n");
        vTaskDelete(*xHandle);
        *xHandle = NULL;
    }

    switch (taskType)
    {
    case RESTART_TASK:
    {
        // Create the task, storing the handle.
        xReturned = xTaskCreate(
            restartTask,       // Function that implements the task.
            TASK_NAME,         // Text name for the task.
            DEFAULT_STACKSIZE, // Stack size in words, not bytes.
            (void *)param,     // Parameter passed into the task.
            DEFAULT_PRIORITY,  // Priority at which the task is created.
            xHandle);          // Used to pass out the created task's handle.
                               // We are passing it as a pointer, so is not neccesary anymore
        break;
    }
    case LOOP_TASK:
    {
        xReturned = xTaskCreate(
            loopTask,
            TASK_NAME,
            DEFAULT_STACKSIZE,
            (void *)param,
            DEFAULT_PRIORITY,
            xHandle);

        break;
    }
    }

    if (xReturned == pdPASS)
    {
        // The task was created. Use the task's handle to delete the task.
        printf("Task created sucessfully\n");
    }
    else
    {
        printf("There was an error creating the task\n");
        return;
    }
}

void apply_sent_data()
{
    if (global_brightness != -1)
        setBrightness(global_brightness);
    for (int i = 0; i < num_leds; i++)
    {
        if (leds_json[i].brightness != -1)
        {
            color_t prevChange = {0, 0, 0};
            if (leds_json[i].hex_color != -1)
            { // Means than a new color is set
                prevChange = HEXtoRGB(leds_json[i].hex_color);
            }
            else
            { // Means than color remains
                prevChange = getLedColor(i);
            }
            color_t RGB = setBrightnessSingleRGB(prevChange.r, prevChange.g, prevChange.b, leds_json[i].brightness);
            setRGB(i, RGB.r, RGB.g, RGB.b);
        }
        else if (leds_json[i].hex_color != -1 && leds_json[i].brightness == -1)
            setHEX(i, leds_json[i].hex_color);
    }
    if (!isIntermitent || !isFlush){
        if (xHandle!=NULL){
            vTaskDelete(xHandle);
            xHandle=NULL;
            vTaskDelay(DEFAULT_TICK_DELAY);
        }
            
    }
    if (isIntermitent || isFlush)
        createNewTask((isIntermitent&&isFlush)?"INTERMITENT_AND_FLUSH":
            ((isIntermitent)?"INTERMITENT":"FLUSH")
            ,NULL, &xHandle, LOOP_TASK);
}

static void handle_topic(const char *full_topic, const char *subtopic,
                         const char *data, esp_mqtt_client_handle_t client)
{
    // ESP_LOGI(TAG, "Performing action for subtopic: %s, data: %s", subtopic, data);

    vTaskDelay(DEFAULT_TICK_DELAY);
    if (strcmp(subtopic, DEFAULT_FUNCTION) == 0)
    {
        if (strcmp(data, CLEAR) == 0)
        {
            clearLeds_broker();
            sendData();
        }
        else if (strcmp(data, SPANISH_FLAG) == 0)
        {
            clearLeds_broker();
            sendData();
            spanish_flag();
        }
        else if (strcmp(data, ITALIAN_FLAG) == 0)
        {
            clearLeds_broker();
            sendData();
            italian_flag();
        }
        else if (strcmp(data, ANDALUSIAN_FLAG) == 0)
        {
            clearLeds_broker();
            sendData();
            andalusia_flag();
        }
        else if (strcmp(data, HAPPY_FACE) == 0)
        {
            clearLeds_broker();
            sendData();
            happy_face();
        }
        else if (strcmp(data, RAINBOW) == 0)
        {
            clearLeds_broker();
            sendData();
            rainbow();
        }
        else
        {
            const char *base_topic_full_to_empty = "full_to_empty/color=";

            if (strncmp(data, base_topic_full_to_empty, strlen(base_topic_full_to_empty)) == 0)
            {
                const char *HEX_full_to_empty_char = data + strlen(base_topic_full_to_empty);
                const char *hex_without_hash = HEX_full_to_empty_char + 1;
                printf("PRE: %s", hex_without_hash);
                uint32_t HEX_value = (uint32_t)strtol(hex_without_hash, NULL, 16);
                printf("POST: %li", HEX_value);
                full_to_empty(HEX_value);
            }
            else
            {
                ESP_LOGW(TAG, "Received data does not match expected. See MQTT_BROKER.H");
                return;
            }
        }
    }
    else if (strcmp(subtopic, JSON_FUNCTION) == 0)
    {
        ESP_LOGI(TAG, "Performing JSON action");
        get_data_json(data);
        apply_sent_data();
        sendData();
    }
    else
        ESP_LOGE(TAG, "NOT EXPECTED FORMAT");
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        mqtt_client_already_started = true;
        esp_mqtt_client_subscribe(client, TOPIC_BASE_URI_NO_WILDCARD, 0);
        esp_mqtt_client_subscribe(client, TOPIC_BASE_URI, 0);
        ESP_LOGI(TAG, "Subscribed to base topic: %s", TOPIC_BASE_URI);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "MQTT_EVENT_DISCONNECTED");
        createNewTask("RESTART_MQTT", client, &xHandle, RESTART_TASK);
        vTaskDelay(DEFAULT_TICK_DELAY);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
        if (mqtt_client_already_started)
        {
            mqtt_client_already_started = false;
            createNewTask("RESTART_MQTT", client, &xHandle, RESTART_TASK);
            vTaskDelay(DEFAULT_TICK_DELAY);
        }
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");

        // Extract the full topic
        char full_topic[100];
        snprintf(full_topic, sizeof(full_topic), "%.*s", event->topic_len, event->topic);
        ESP_LOGI(TAG, "Full topic: %s", full_topic);

        // Extract the subtopic
        const char *base_topic = "topic/lamp/";
        const char *subtopic = full_topic + strlen(base_topic);
        if (strncmp(full_topic, base_topic, strlen(base_topic)) == 0)
        {
            subtopic = full_topic + strlen(base_topic);
        }
        else
        {
            ESP_LOGW(TAG, "Received topic doesn't match expected base topic");
            return;
        }

        char *data;
        size_t buffer = 1;
        // Extract the data
        if (strcmp(subtopic, JSON_FUNCTION) == 0)
        {
            buffer = 1500;
            data = malloc(buffer);
        }
        else
        {
            buffer = 250;
            data = malloc(buffer);
        }

        snprintf(data, buffer, "%.*s", event->data_len, event->data);
        ESP_LOGI(TAG, "Data: %s", data);

        handle_topic(full_topic, subtopic, data, client);
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");
        ESP_LOGI(TAG, "Subscribed to topic, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED");
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED");
        break;

    case MQTT_EVENT_BEFORE_CONNECT:
        ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
        break;

    case MQTT_EVENT_DELETED:
        ESP_LOGI(TAG, "MQTT_EVENT_DELETED");
        break;

    case MQTT_USER_EVENT:
        ESP_LOGI(TAG, "MQTT_USER_EVENT");
        break;
    default:
        ESP_LOGW(TAG, "Unknown event ID: %" PRId32, event_id);
        break;
    }
}

void mqtt_init(char *BROKER_URI, char *AUTH_USR, char *AUTH_PWD, int numLeds)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = BROKER_URI},
        .credentials = {.username = AUTH_USR, // Set MQTT username
                        .authentication = {
                            .password = AUTH_PWD // Set MQTT password
                        }},
        .network.disable_auto_reconnect = true, // Disable auto-reconnect
        .network.timeout_ms = 300               // Reduce keep-alive timeout
    };
    ESP_LOGI(TAG, "STARTING MQTT CONFIGURATION");
    num_leds = numLeds;
    init_led_json();
    vTaskDelay(DEFAULT_TICK_DELAY);
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}