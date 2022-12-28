long _lastStateCheck = 0;
volatile int _prevState = HIGH;
long _prevMillis = 0;

void setTimeout(void (*callback)(), int interval) {
  unsigned long currentMillis = millis();

  if (currentMillis - _prevMillis > interval) {
    _prevMillis = currentMillis;
    callback();
  }
}

bool didStateChange(int state) {
  return _prevState != state;
}

void listenForStateChange(int (*getState)(), void (*onStateChange)(int state), int interval) {
  unsigned long currentStateCheck = millis();

  if (currentStateCheck - _lastStateCheck > interval) {
    _lastStateCheck = currentStateCheck;
    int state = getState();

    if (didStateChange(state)) {
      _prevState = state;
      onStateChange(state);
    }
  }
}
