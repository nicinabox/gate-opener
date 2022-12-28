int _prevState = -1;

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

void listenForStateChange(int (*getState)(), void (*onStateChange)(int state), int initialState) {
  int state = getState();

  if (_prevState == -1) _prevState = initialState;

  if (_prevState != state)
  {
    _prevState = state;
    onStateChange(state);
  }
}
