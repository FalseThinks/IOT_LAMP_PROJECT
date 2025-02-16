#include "Paproka_LED_Lamp.h"
#include "LED_Default_Functions.h"
#include "mqtt_client.h"

#define TOPIC_BASE_URI_NO_WILDCARD "topic/lamp/"
// # is used as wildcard for receiving new messages
#define TOPIC_BASE_URI "topic/lamp/#"

//-- DEFAULT FUNCTIONS

//Example: lamp/default_functions/ + flush/flush_one/color=HEX
#define DEFAULT_FUNCTION "default_functions/"

#define SPANISH_FLAG "flags/spanish_flag"
#define ANDALUSIA_FLAG "flags/andalusian_flag"
#define ITALIAN_FLAG "flags/italian_flag"

#define HAPPY_FACE "happy_face"
#define RAINBOW "rainbow"

#define FULL_TO_EMPTY "full_to_empty/color=HEX"

#define FLUSH_ONE "flush/flush_one/color=HEX&amount=4"
#define FLUSH_TWO "flush/flush_two/color1=HEX&color2=HEX2&amount=4"
#define FLUSH_THREE "flush/flush_three/color1=HEX&color2=HEX2&color3=HEX3&amount=4"

//-- END OF DEFAULT FUNCTIONS

#define SET_COLOR "set_color/" //LED_NUMBER + HEX
#define FILL_COLOR "fill_color/" //HEX + STARTING_LED + COUNT
#define SET_BRIGHTNESS "set_brightness/" //VALUE
#define SET_BRIGHTNESS_UNIQUE "set_brightness_unique/" //LED_NUMBER + VALUE

/**
 * Starts MQTT server.
 * @param BROKER_URI Broker endpoint.
 * @param AUTH_USR User login authorization.
 * @param AUTH_PWD Password login authorization.
 * @author paproka
 */
void mqtt_init(char *BROKER_URI, char *AUTH_USR, char *AUTH_PWD);
