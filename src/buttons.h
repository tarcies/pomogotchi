#ifndef BUTTONS_H
#define BUTTONS_H

enum Mode {
  MODE_NORMAL,
  MODE_MUSIC,
  MODE_TASKS
};

extern Mode currentMode;

void initButtons();
void updateButtons();

#endif