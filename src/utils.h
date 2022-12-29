int _prevState = -1;
long prevMillis = 0;
long timeoutDelay = 0;

void setTimeout(void (*callback)(), int delay) {
  if (!timeoutDelay) {
    timeoutDelay = millis() + delay;
  }

  if (millis() >= timeoutDelay) {
    timeoutDelay = 0;
    callback();
  }
}

void listenForStateChange(int (*getState)(), void (*onStateChange)(int state), int debounceInterval) {
  unsigned long currentMillis = millis();

  if (currentMillis - prevMillis > debounceInterval)
  {
    prevMillis = currentMillis;
    int state = getState();

    if (_prevState != state)
    {
      _prevState = state;
      onStateChange(state);
    }
  }
}
