#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>

struct Task {
  String name;
  bool done;
};

extern Task tasks[50];
extern int taskCount;

void loadTasksFromSD();
void saveTasksToSD();

#endif
