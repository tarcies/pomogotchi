#ifndef POMODORO_H
#define POMODORO_H

void initPomodoro();
void updatePomodoro();
int getPomodoroTime();
bool isPomodoroFocus();
bool isPomodoroRunning();
void playPausePomodoro();
void endPomodoroPhase(bool manual);

#endif