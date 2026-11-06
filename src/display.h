#ifndef DISPLAY_H
#define DISPLAY_H

#include "sd_card.h"
#include "sensors.h"
#include "FS.h"

void initDisplay();
void startDisplay();
void updateDisplay(float temp, float humid, int aqi, String currTime, int pomodoroTimerS, bool pomodoroFocus, bool pomodoroRunning, int songMaxS, bool dark);
void topBar(float temp, float humid, int aqi, String currTime);
void pomodoro(int pomodoroTimerS, bool pomodoroFocus, bool pomodoroRunning);
void updatePomodoroTab(bool darkMode);
void drawTamagotchi();
void drawBmp(const char *filename, int16_t x, int16_t y, bool transparent);
uint16_t read16(File &f);
uint32_t read32(File &f);
void happinessBar();
void newTrack(String name, int maxS);
void updateBotBar();
void botBar();
void updateTrack(int currS);
void togglePlayPauseSymbol();
void setMusicMode(bool show);
void toggleBotHighlight();
void toggleVolBar();
void setTaskMode(bool show);
void startTaskTab();
void updateTaskTab(bool newPage);
void changeSelectedTask(bool up);
void checkTask();

#endif
