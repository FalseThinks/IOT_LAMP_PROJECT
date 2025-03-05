#include "Paproka_LED_Lamp.h"
#include "LED_Default_Functions.h"
#include "mqtt_client.h"
#include <cJSON.h>

typedef enum
{
    RESTART_TASK,
    LOOP_TASK
} task_type_t;

typedef struct
{
    uint32_t hex_color;
    int brightness;
} led_status_json_t;

#define TOPIC_BASE_URI_NO_WILDCARD "topic/lamp/"
// # is used as wildcard for receiving new messages
#define TOPIC_BASE_URI "topic/lamp/#"

//-- DEFAULT FUNCTIONS
// Example: lamp/default_functions/ + flags/spanish_flag/
#define DEFAULT_FUNCTION "default_functions/"

#define SPANISH_FLAG "flags/spanish_flag"
#define ANDALUSIAN_FLAG "flags/andalusian_flag"
#define ITALIAN_FLAG "flags/italian_flag"
#define HAPPY_FACE "happy_face"
#define RAINBOW "rainbow"
#define FULL_TO_EMPTY "full_to_empty/color=HEX" // HEX Format: #RRGGBB
#define CLEAR "clear"

//-- END OF DEFAULT FUNCTIONS
#define JSON_FUNCTION "json_function/"

#define SET_COLOR "set_color/"                         // LED_NUMBER + HEX
#define FILL_COLOR "fill_color/"                       // HEX + STARTING_LED + COUNT
#define SET_BRIGHTNESS "set_brightness/"               // VALUE
#define SET_BRIGHTNESS_UNIQUE "set_brightness_unique/" // LED_NUMBER + VALUE

/**
 * Starts MQTT server.
 * @param BROKER_URI Broker endpoint.
 * @param AUTH_USR User login authorization.
 * @param AUTH_PWD Password login authorization.
 * @param numLeds Amount of lamp's leds.
 * @author paproka
 */
void mqtt_init(char *BROKER_URI, char *AUTH_USR, char *AUTH_PWD, int numLeds);

// JSON File structure detailed below
/*
    {
        "leds":
        {
            "0":
            {
                "c" = "#RRGGBB", <- color in hex format
                "b" = "50" <- brightness (0-255)
            },
            "1":
            {
                "c" = "-1" <- Previous color remains
                "b" = "-1" <- Previous brightness remains
            },
            ...
            "23":
            {
                "c" = "#RRGGBB",
                "b" = "-1"
            }
        }
        "is_flush" = true/false,
        "is_intermitent" = true/false,
        "global_b" = "255"
    }

*/