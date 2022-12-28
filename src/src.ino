#include <ESP8266WiFi.h>
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

void setState(int nextState) {
    prevState = state;
    state = nextState;
    pubclient.publish(MQTT_TOPIC_CURRENT_STATE, (const char *) state);
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

    default:
      return currentState;
  }
}

int getNextState(int currentState) {
  switch(currentState) {
    case DOOR_CLOSED:
      return DOOR_OPENING;

    case DOOR_OPEN:
      return DOOR_CLOSING;

    case DOOR_CLOSING:
      return DOOR_CLOSED;

    case DOOR_OPENING:
      return DOOR_OPEN;

    default:
      return DOOR_UNKNOWN;
  }
}

void onOpen() {
  if (state == DOOR_OPENING)
  {
      setState(DOOR_OPEN);
  }
}

void onSensorChange(int sensorState) {
  // Opened from HomeKit
  if (targetState == DOOR_OPEN)
    {
      // Defer open state
      setState(getTransitionState(state));
      return;
  }

  // Opened externally
  if (state == DOOR_CLOSED)
  {
    targetState = sensorState;
    pubclient.publish(MQTT_TOPIC_TARGET_STATE, (const char *) targetState);
    setState(getTransitionState(state));
    return;
  }

  // Closed externally or from HomeKit
  if (sensorState == DOOR_CLOSED) {
    targetState = sensorState;
    pubclient.publish(MQTT_TOPIC_TARGET_STATE, (const char *)targetState);
    setState(sensorState);
    return;
  }
}

void onMessageReceived(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (topic == MQTT_TOPIC_TARGET_STATE) {
      targetState = (int) payload;
      cycleRelay();
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

void reconnect() {
  // Loop until we"re reconnected
  while (!pubclient.connected()) {
    Serial.print("Trying MQTT connection...");
    // Attempt to connect
    if (pubclient.connect(HOSTNAME)) {
      Serial.println("connected");
      pubclient.publish(MQTT_TOPIC_CONNECTED, "true");
      pubclient.subscribe(MQTT_TOPIC_TARGET_STATE);
    } else {
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

    awaitWifiConnected();
    pubclient.setCallback(onMessageReceived);

    pinMode(LED_PIN, OUTPUT);
    setLEDState();

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    pinMode(SENSOR_CLOSED_PIN, INPUT_PULLUP);

    state = readSensor();
    prevState = state;
    targetState = state;

    onSensorChange(state);
}

void loop() {
    pubclient.loop();

    if (!pubclient.connected()) {
        reconnect();
    }

    listenForStateChange(&readSensor, &onSensorChange, 1000);

    if (state == DOOR_OPENING) {
      setTimeout(&onOpen, DOOR_OPENING_TIME_MS);
    }
}
