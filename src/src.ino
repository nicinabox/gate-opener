#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#else
#error Only ESP32 or ESP8266 are supported.
#endif

#include <PubSubClient.h>
#include "utils.h"
#include "config.h"

WiFiClient wifiClient;
PubSubClient pubclient(MQTT_HOST, MQTT_PORT, wifiClient);

const int DOOR_OPEN = 0;
const int DOOR_CLOSED = 1;
const int DOOR_OPENING = 2;
const int DOOR_CLOSING = 3;
const int DOOR_STOPPED = 4;
const int DOOR_UNKNOWN = 5;

volatile int prevState = DOOR_UNKNOWN;
volatile int state = DOOR_UNKNOWN;
volatile int targetState = DOOR_UNKNOWN;

void setRelayState(int state) {
    digitalWrite(RELAY_PIN, state);
}

void setLEDState() {
    int sensorState = digitalRead(SENSOR_CLOSED_PIN);
    digitalWrite(LED_PIN, sensorState);
}

void publish(char * topic, int value) {
    char payload[16];
    itoa(value, payload, 10);

    Serial.print("Publish message [");
    Serial.print(topic);
    Serial.print("] ");
    Serial.println(payload);

    pubclient.publish(topic, payload);
}

void setState(int nextState) {
    prevState = state;
    state = nextState;
    setLEDState();
    publish(MQTT_TOPIC_CURRENT_STATE, state);
}

void cycleRelay() {
    setRelayState(HIGH);
    delay(100);
    setRelayState(LOW);
}

int readSensor() {
    // 1 - switch open
    // 0 - switch closed
    int value = digitalRead(SENSOR_CLOSED_PIN);

    // invert to match homekit states
    return 1 - value;
}

int getTargetState(int currentState) {
  switch(currentState) {
    case DOOR_CLOSED:
      return DOOR_OPEN;

    case DOOR_OPEN:
      return DOOR_CLOSED;

    case DOOR_CLOSING:
    case DOOR_OPENING:
      return DOOR_STOPPED;

    // Reverse direction
    case DOOR_STOPPED:
      if (prevState == DOOR_OPENING) return DOOR_CLOSED;
      if (prevState == DOOR_CLOSING) return DOOR_OPEN;

    default:
      return DOOR_UNKNOWN;
  }
}

int getTransitionState(int currentState) {
  switch(currentState) {
    case DOOR_CLOSED:
      return DOOR_OPENING;

    case DOOR_OPEN:
      return DOOR_CLOSING;

    case DOOR_STOPPED:
      if (prevState == DOOR_CLOSING)
        return DOOR_OPENING;
      if (prevState == DOOR_OPENING)
        return DOOR_CLOSING;

    default:
      return currentState;
  }
}

void onOpen() {
  if (state == DOOR_OPENING)
  {
      setState(DOOR_OPEN);
  }
}

void onSensorChange(int sensorState) {
  targetState = sensorState;
  publish(MQTT_TOPIC_GET_TARGET_STATE, sensorState);

  // Defer open state
  if (state == DOOR_CLOSED && sensorState == DOOR_OPEN) {
      setState(getTransitionState(state));
      return;
  }

  setState(sensorState);
}

void onMessageReceived(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message received [");
  Serial.print(topic);
  Serial.print("] ");

  String message = "";
  for (size_t i=0; i<length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  if (strcmp(topic, MQTT_TOPIC_SET_TARGET_STATE) == 0)
  {
    int nextTargetState = message.toInt();
    if (nextTargetState != targetState) {
      targetState = nextTargetState;
      cycleRelay();
    }
  }
}

void awaitWifiConnected() {
  Serial.setDebugOutput(true);
  Serial.println("Trying to connect " + String(WIFI_SSID));

  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 9);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println(WiFi.localIP());
}

void awaitMQTTConnected() {
  // Loop until reconnected
  while (!pubclient.connected()) {
    Serial.print("Trying MQTT connection...");

    bool connected = pubclient.connect(
        MQTT_CLIENT_ID,
        MQTT_USER,
        MQTT_PASS,
        MQTT_LAST_WILL_TOPIC,
        MQTT_LAST_WILL_QOS,
        MQTT_LAST_WILL_RETAIN,
        MQTT_LAST_WILL_MESSAGE);

    // Attempt to connect
    if (connected)
    {
      Serial.println("connected");
      pubclient.publish(MQTT_TOPIC_CONNECTED, "true");
      pubclient.subscribe(MQTT_TOPIC_SET_TARGET_STATE);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(pubclient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
    Serial.begin(115200);

    pubclient.setCallback(onMessageReceived);

    awaitWifiConnected();
    awaitMQTTConnected();

    pinMode(LED_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(SENSOR_CLOSED_PIN, SENSOR_PIN_MODE);

    state = readSensor();
    prevState = state;
    targetState = state;

    digitalWrite(RELAY_PIN, LOW);
    setLEDState();
}

void loop() {
    pubclient.loop();

    if (!pubclient.connected()) {
        awaitMQTTConnected();
    }

    listenForStateChange(&readSensor, &onSensorChange, 1000);

    if (state == DOOR_OPENING) {
      setTimeout(&onOpen, DOOR_OPENING_TIME_MS);
    }
}
