#include "tasks.h"
#include "sd_card.h"
#include <ArduinoJson.h>

Task tasks[50];
int taskCount = 0;

void loadTasksFromSD() {
  File file = openFileMode("/web/tasks.json", FILE_READ);
  if (!file) {
    Serial.println("Failed to open /web/tasks.json");
    return;
  }

  // Read file into string
  String jsonString;
  while (file.available()) {
    jsonString += (char)file.read();
  }
  file.close();

  // Parse JSON
  StaticJsonDocument<4096> doc;  // adjust size for your file size
  DeserializationError error = deserializeJson(doc, jsonString);
  if (error) {
    Serial.println("Failed to parse JSON");
    return;
  }

  JsonArray arr = doc["tasks"].as<JsonArray>();
  taskCount = 0;
  for (JsonObject obj : arr) {
    if (taskCount >= 50) break;
    tasks[taskCount].name = (const char*)obj["title"];
    tasks[taskCount].done = obj["done"];
    taskCount++;
  }

  return;
}

void saveTasksToSD() {
  StaticJsonDocument<4096> doc;
  JsonArray arr = doc.createNestedArray("tasks");

  for (int i = 0; i < taskCount; i++) {
    JsonObject obj = arr.createNestedObject();
    obj["title"] = tasks[i].name;
    obj["done"] = tasks[i].done;
  }

  File file = openFileMode("/web/tasks.json", FILE_WRITE);
  if (!file) {
    Serial.println("Failed to write tasks.json");
    return;
  }
  serializeJsonPretty(doc, file);
  file.close();
  Serial.println("Tasks saved!");
}
