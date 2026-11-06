#include "esp32-hal.h"
#include "config.h"
#include "buttons.h"
#include "display.h"
#include "pomodoro.h"
#include "sound.h"

#define LONG_PRESS_MS 800

Mode currentMode = MODE_NORMAL;

struct ButtonState {
  bool pressed = false;
  unsigned long pressTime = 0;
};

ButtonState btnLeft, btnMid, btnRight;
bool longTrig = false;

void handleButtonAction(int btn, bool longPress) {
  switch (currentMode) {
    case MODE_NORMAL:
      switch (btn) {
        case BTN_PIN_L:
          if (longPress) { // Left - Hold
            Serial.println("Playlist");

          } else { // Left - Tap
            Serial.println("Music Mode");
            currentMode = MODE_MUSIC;
            setMusicMode(true);
          }
          break;

        case BTN_PIN_M:
          if (longPress) { // Mid - Hold
            Serial.println("Cancel Timer");
            endPomodoroPhase(true);
          } else { // Mid - Tap
            Serial.println("Pause/Play Timer");
            playPausePomodoro();
          }
          break;

        case BTN_PIN_R:
          if (longPress) { // Right - Hold
            Serial.println("Task Lists");

          } else { // Right - Tap
            Serial.println("Task Mode");
            currentMode = MODE_TASKS;
            setTaskMode(true);
          }
          break;
      }
      break;

    case MODE_MUSIC:
      switch (btn) {
        case BTN_PIN_L:
          if (longPress) {
            Serial.println("Previous Track");
            changeTrack();
          } else {
            Serial.println("Vol-");
            changeVol(false);
          }
          break;

        case BTN_PIN_M:
          if (longPress) {
            Serial.println("Normal Mode");
            currentMode = MODE_NORMAL;
            setMusicMode(false);
          } else {
            Serial.println("Play/Pause Music");
            playPauseMusic();
          }
          break;

        case BTN_PIN_R:
          if (longPress) {
            Serial.println("Next Track");
            changeTrack();
          } else {
            Serial.println("Vol+");
            changeVol(true);
          }
          break;
      }
      break;

    case MODE_TASKS:
      switch (btn) {
        case BTN_PIN_L:
          if (longPress) {
            Serial.println("Previous List");

          } else {
            Serial.println("Previous Task");
            changeSelectedTask(false);
          }
          break;

        case BTN_PIN_M:
          if (longPress) {
            Serial.println("Normal Mode");
            currentMode = MODE_NORMAL;
            setTaskMode(false);

          } else {
            Serial.println("Un/Check Task");
            checkTask();
          }
          break;

        case BTN_PIN_R:
          if (longPress) {
            Serial.println("Next List");

          } else {
            Serial.println("Next Task");
            changeSelectedTask(true);
          }
          break;
      }
      break;
  }
}

void checkButton(int pin, ButtonState &btn) {
  bool state = digitalRead(pin) == LOW; // assuming pull-up

  if (state && !btn.pressed) {
    btn.pressed = true;
    btn.pressTime = millis();
    longTrig = false;
  } else if (state && (millis() - btn.pressTime) >= LONG_PRESS_MS && !longTrig) {
    // Long press
    longTrig = true; // Prevents double firing and 'Short press' from firing after button release
    handleButtonAction(pin, true);
  } else if (!state && btn.pressed && !longTrig) {
    // Short press
    handleButtonAction(pin, false);
    btn.pressed = false;
  } else if (!state && btn.pressed) {
    btn.pressed = false;
  }
}

void initButtons() {
  pinMode(BTN_PIN_L, INPUT_PULLUP);
  pinMode(BTN_PIN_M, INPUT_PULLUP);
  pinMode(BTN_PIN_R, INPUT_PULLUP);
}

void updateButtons() {
  checkButton(BTN_PIN_L, btnLeft);
  checkButton(BTN_PIN_M, btnMid);
  checkButton(BTN_PIN_R, btnRight);
}
