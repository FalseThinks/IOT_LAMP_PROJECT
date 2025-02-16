#include "mqtt_broker.h"
#include "inttypes.h"

static const char *TAG = "MQTT_CLIENT";

#define DEFAULT_STACKSIZE 4096     // Stack size needed for MQTT tasks
#define DEFAULT_PRIORITY tskIDLE_PRIORITY              // Lowest possible task priority
#define DEFAULT_TICK_DELAY (1000 / portTICK_PERIOD_MS) // Time for delay
static TaskHandle_t xHandle = NULL;                    // Task handle

static bool mqtt_client_already_started = false;

void task(void *pvParameters)
{
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)pvParameters;

    esp_mqtt_client_stop(client);
    mqtt_client_already_started = false;
    // Wait for a short delay (e.g., 5 seconds)
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    // Restart the MQTT client
    ESP_LOGI(TAG, "Restarting MQTT client...");
    esp_mqtt_client_start(client);
    mqtt_client_already_started = true;

    // Delete the task after completion
    vTaskDelete(NULL);
}

void createNewRestartTask(const char *const TASK_NAME, esp_mqtt_client_handle_t param, TaskHandle_t *xHandle)
{
    BaseType_t xReturned;

    // Create the task, storing the handle.
    xReturned = xTaskCreate(
        task,              // Function that implements the task.
        TASK_NAME,         // Text name for the task.
        DEFAULT_STACKSIZE, // Stack size in words, not bytes.
        (void *)param,     // Parameter passed into the task.
        DEFAULT_PRIORITY,  // Priority at which the task is created.
        xHandle);          // Used to pass out the created task's handle.
                           // We are passing it as a pointer, so is not neccesary anymore.

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

static void handle_topic(const char *full_topic, const char *subtopic,
                         const char *data, esp_mqtt_client_handle_t client)
{
    ESP_LOGI(TAG, "Performing action for subtopic: %s, data: %s", subtopic, data);

    if (strcmp(subtopic, DEFAULT_FUNCTION) == 0)
    {
        if (strcmp(data, SPANISH_FLAG) == 0)
        {
            spanish_flag();
        }
        else if (strcmp(data, ANDALUSIA_FLAG) == 0)
        {
            andalusia_flag();
        }
        else if (strcmp(data, HAPPY_FACE) == 0)
        {
            happy_face();
        }
        else if (strcmp(data, RAINBOW) == 0)
        {
            rainbow();
        }
        else
        {
            // HANDLE WITH DATA INPUTS
        }
    }
    else
    { // Other stuff
    }
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
        if (mqtt_client_already_started)
        {
            mqtt_client_already_started = false;
            createNewRestartTask("RESTART_MQTT", client, &xHandle);
        }

        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
        if (mqtt_client_already_started)
        {
            mqtt_client_already_started = false;
            createNewRestartTask("RESTART_MQTT", client, &xHandle);
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

        // Extract the data
        char data[100];
        snprintf(data, sizeof(data), "%.*s", event->data_len, event->data);
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

void mqtt_init(char *BROKER_URI, char *AUTH_USR, char *AUTH_PWD)
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

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}
