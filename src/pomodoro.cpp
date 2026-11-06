#include "config.h"
#include "pomodoro.h"
#include "tamagotchi.h"
#include "sound.h"

int timerS;
int cycleCount;
bool running;
bool focus;
bool endCycle;
unsigned long frameDuration;

void initPomodoro() {
  running = false;
  focus = true;
  timerS = focus ? FOCUS_PERIOD_M*60 : REST_PERIOD_M*60;
  cycleCount = 0;
  frameDuration = 0;
}

void updatePomodoro() {
  if (running) {
    if (millis() - frameDuration >= 50) {
      frameDuration = millis();
      if (timerS > 0) {
        timerS--;
      } else {
        endPomodoroPhase(false);
      }
    }
  }
}

int getPomodoroTime() {
  return timerS;
}

bool isPomodoroFocus() {
  return focus;
}

bool isPomodoroRunning() {
  return running;
}

void playPausePomodoro() {
  running = !running;
  if (running) {
    if (!isPlaying()) playPauseMusic();
    setEmotion(focus ? FOCUS : SLEEP);
  } else {
    if (isPlaying()) playPauseMusic();
    setEmotion(NEUTRAL);
  }
}

void endPomodoroPhase(bool manual) {
  if (focus) {
    tamagotchiHappiness(!manual);
    setEmotion(!manual ? EXCITED : ANGRY);
  } else {
    setEmotion(!manual ? RELAX : TIRED);
  }
  
  running = false;
  focus = !focus;

  if (focus) { // Next cycle is Focus
    timerS = FOCUS_PERIOD_M*60;
  } else { // Next cycle is Rest
    cycleCount++;
    timerS = (cycleCount < 4) ? REST_PERIOD_M*60 : LONG_REST_PERIOD_M*60;
    if (cycleCount == 4) cycleCount = 0;
  }
  
  if (isPlaying()) playPauseMusic();
  setPlayNotif();
}
