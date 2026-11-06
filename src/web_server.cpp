#include "web_server.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "sd_card.h"
#include "tasks.h"
#include "display.h"

WebServer server(80);
const char* TASKS_FILE = "/web/tasks.json";
String currentTheme = "light";

// ----------- Mode Functions -----------

void setTheme(String mode) {
  currentTheme = mode;
  File file = openFileMode("/web/theme.txt", FILE_WRITE);
  if (file) {
    file.print(currentTheme);
    file.close();
  }
}

void loadThemeFromSD() {
  if (!fileExists("/web/theme.txt")) return;
  File file = openFileMode("/web/theme.txt", FILE_READ);
  if (file) {
    currentTheme = file.readStringUntil('\n');
    file.close();
    currentTheme.trim();
  }
}

String getTheme() {
  return currentTheme;
}

// ---------- Utility Functions ----------

String readFileToString(const char* path) {
  File file = openFileMode(path, FILE_READ);
  if (!file) return "";
  String s;
  while (file.available()) s += (char)file.read();
  file.close();
  return s;
}

bool writeStringToFile(const char* path, const String &data) {
  File file = openFileMode(path, FILE_WRITE);
  if (!file) return false;
  file.print(data);
  file.close();
  loadTasksFromSD();
  return true;
}

void ensureTasksFile() {
  if (!fileExists(TASKS_FILE)) {
    DynamicJsonDocument doc(1024);
    doc["nextId"] = 1;
    doc["tasks"] = JsonArray();
    String out;
    serializeJson(doc, out);
    writeStringToFile(TASKS_FILE, out);
    Serial.println("Created new /tasks.json");
  }
}

bool loadTasks(DynamicJsonDocument &doc) {
  String s = readFileToString(TASKS_FILE);
  if (s.isEmpty()) return false;
  return deserializeJson(doc, s) == DeserializationError::Ok;
}

bool saveTasks(DynamicJsonDocument &doc) {
  String out;
  serializeJson(doc, out);
  return writeStringToFile(TASKS_FILE, out);
}

// ---------- REST Handlers ----------

void handleGetTasks() {
  DynamicJsonDocument doc(4096);
  if (!loadTasks(doc)) {
    server.send(500, "application/json", "{\"error\":\"Failed to read tasks\"}");
    return;
  }
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleAddTask() {
  String body = server.arg("plain");
  DynamicJsonDocument doc(4096);
  if (!loadTasks(doc)) {
    server.send(500, "application/json", "{\"error\":\"Failed to read tasks\"}");
    return;
  }

  DynamicJsonDocument tmp(512);
  if (deserializeJson(tmp, body)) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  const char* title = tmp["title"] | "";
  const char* note = tmp["note"] | "";
  bool done = tmp["done"] | false;

  int id = doc["nextId"] | 1;
  JsonObject task = doc["tasks"].createNestedObject();
  task["id"] = id;
  task["title"] = title;
  task["note"] = note;
  task["done"] = done;
  doc["nextId"] = id + 1;

  if (!saveTasks(doc)) {
    server.send(500, "application/json", "{\"error\":\"Failed to save\"}");
    return;
  }

  server.send(201, "application/json", "{\"status\":\"ok\"}");
}

void handleUpdateTask() {
  String body = server.arg("plain");
  DynamicJsonDocument tmp(512);
  if (deserializeJson(tmp, body)) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  int id = tmp["id"] | -1;
  if (id < 0) {
    server.send(400, "application/json", "{\"error\":\"Missing ID\"}");
    return;
  }

  DynamicJsonDocument doc(4096);
  if (!loadTasks(doc)) {
    server.send(500, "application/json", "{\"error\":\"Failed to read tasks\"}");
    return;
  }

  bool found = false;
  for (JsonObject t : doc["tasks"].as<JsonArray>()) {
    if (t["id"] == id) {
      if (tmp.containsKey("title")) t["title"] = tmp["title"].as<const char*>();
      if (tmp.containsKey("note")) t["note"] = tmp["note"].as<const char*>();
      if (tmp.containsKey("done")) t["done"] = tmp["done"].as<bool>();
      found = true;
      break;
    }
  }

  if (!found) {
    server.send(404, "application/json", "{\"error\":\"Not found\"}");
    return;
  }

  if (!saveTasks(doc)) {
    server.send(500, "application/json", "{\"error\":\"Failed to save\"}");
    return;
  }

  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleDeleteTask() {
  if (!server.hasArg("id")) {
    server.send(400, "application/json", "{\"error\":\"Missing ID\"}");
    return;
  }

  int id = server.arg("id").toInt();
  DynamicJsonDocument doc(4096);
  if (!loadTasks(doc)) {
    server.send(500, "application/json", "{\"error\":\"Failed to read tasks\"}");
    return;
  }

  JsonArray tasks = doc["tasks"].as<JsonArray>();
  DynamicJsonDocument out(4096);
  out["nextId"] = doc["nextId"];
  JsonArray newTasks = out.createNestedArray("tasks");
  bool removed = false;

  for (JsonObject t : tasks) {
    if (t["id"] == id) {
      removed = true;
      continue;
    }
    JsonObject nt = newTasks.createNestedObject();
    nt["id"] = t["id"];
    nt["title"] = (const char*)t["title"];
    nt["note"] = (const char*)t["note"];
    nt["done"] = t["done"];
  }
  
  if (!removed) {
    server.send(404, "application/json", "{\"error\":\"Not found\"}");
    return;
  }

  if (!saveTasks(out)) {
    server.send(500, "application/json", "{\"error\":\"Failed to save\"}");
    return;
  }

  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

// ---------- Static file server ----------

void handleFileRequest() {
  String path = "/web" + server.uri();
  if (path.endsWith("/")) path += "index.html";

  String type = "text/plain";
  if (path.endsWith(".html")) type = "text/html";
  else if (path.endsWith(".css")) type = "text/css";
  else if (path.endsWith(".js")) type = "application/javascript";
  else if (path.endsWith(".json")) type = "application/json";

  if (!fileExists(path.c_str())) {
    server.send(404, "text/plain", "Not found");
    return;
  }

  File file = openFile(path.c_str());
  server.streamFile(file, type);
  file.close();
}

// ---------- Initialization ----------

void initWebServer() {
  ensureTasksFile();

  server.on("/api/tasks", HTTP_GET, handleGetTasks);
  server.on("/api/tasks", HTTP_POST, handleAddTask);
  server.on("/api/tasks", HTTP_PUT, handleUpdateTask);
  server.on("/api/tasks", HTTP_DELETE, handleDeleteTask);
  server.on("/setTheme", HTTP_GET, []() {
  if (server.hasArg("mode")) {
    String mode = server.arg("mode");
    setTheme(mode);
    Serial.println("Theme changed to: " + mode);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing mode");
  }
});

server.on("/getTheme", HTTP_GET, []() {
  server.send(200, "text/plain", getTheme());
});

  server.onNotFound(handleFileRequest);

  server.begin();
  Serial.println("Web server started");
}

// ---------- Main loop handler ----------

void handleWebServer() {
  server.handleClient();
}
