int const INTERVAL = 1000;
long lastStateCheck = 0;
volatile int prevState = HIGH;
long prevMillis = 0;

void setTimeout(void (*callback)(), int interval) {
  unsigned long currentMillis = millis();

  if (currentMillis - prevMillis > interval) {
    prevMillis = currentMillis;
    callback();
  }
}

bool didStateChange(int state) {
  return prevState != state;
}

void listenForStateChange(int (*getState)(), void (*onStateChange)(int state)) {
  unsigned long currentStateCheck = millis();

  if (currentStateCheck - lastStateCheck > INTERVAL) {
    lastStateCheck = currentStateCheck;
    int state = getState();

    if (didStateChange(state)) {
      prevState = state;
      onStateChange(state);
    }
  }
}
