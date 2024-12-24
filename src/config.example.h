#if defined(ESP32)
#define RELAY_PIN 13
#define LED_PIN 2
#define SENSOR_CLOSED_PIN 5
#define SENSOR_PIN_MODE INPUT
#endif

#if defined(ESP8266)
#define RELAY_PIN 12
#define LED_PIN 13
#define SENSOR_CLOSED_PIN 5
#define SENSOR_PIN_MODE INPUT_PULLUP
#endif

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define HOSTNAME ""

#define MQTT_HOST "0.0.0.0"
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASS ""
#define MQTT_KEEP_ALIVE 15
#define MQTT_CLIENT_ID HOSTNAME

#define MQTT_LAST_WILL_TOPIC "stat/connected"
#define MQTT_LAST_WILL_QOS 0
#define MQTT_LAST_WILL_RETAIN true
#define MQTT_LAST_WILL_MESSAGE "false"

#define MQTT_TOPIC_CONNECTED "stat/connected"
#define MQTT_TOPIC_CURRENT_STATE "stat/current"
#define MQTT_TOPIC_SET_TARGET_STATE "stat/set_target"
#define MQTT_TOPIC_GET_TARGET_STATE "stat/get_target"

#define DOOR_OPENING_TIME_MS 10000

#define WDT_TIMEOUT 120
