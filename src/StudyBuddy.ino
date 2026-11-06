#include <Arduino.h>
#include "display.h"
#include "sensors.h"
#include "sound.h"
#include "sd_card.h"
#include "buttons.h"
#include "pomodoro.h"
#include "tamagotchi.h"
#include "wifi_util.h"
#include "web_server.h"
#include "tasks.h"

unsigned long lastSensorUpdate = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastTamagotchiUpdate = 0;
unsigned long lastButtonCheck = 0;

float temp;
float humid;
int aqi;
String currTime;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  initSD();
  loadThemeFromSD();
  initTamagotchi();
  initDisplay();
  connectWiFi();
  initSensors();
  initButtons();
  initPomodoro();
  initMusic();
  initWebServer();
  loadTasksFromSD();

  delay(2000);

  Serial.println("NOW");
  
  startDisplay();
  playRandomTrack();

  xTaskCreatePinnedToCore(TaskSound, "Sound", 4096, NULL, 1, NULL, 1);
}

void loop() {
  unsigned long now = millis();

  if (now - lastSensorUpdate > 2000) {
    temp = getTemperature();
    humid = getHumidity();
    aqi = getAirQuality();
    currTime = getTime();
    lastSensorUpdate = now;
  }

  if (now - lastDisplayUpdate > 50) {
    updateDisplay(temp, humid, aqi, currTime, getPomodoroTime(), isPomodoroFocus(), isPomodoroRunning(), getTrackCurrS(), getDark());
    lastDisplayUpdate = now;
  }
  
  if (now - lastTamagotchiUpdate > 1000) {
    updateTamagotchi(aqi);
    drawTamagotchi();
    lastTamagotchiUpdate = now;
  }

  if (now - lastButtonCheck > 20) {
    updateButtons();
    lastButtonCheck = now;
  }

  updatePomodoro();
  handleWebServer();
}

void TaskSound(void *pvParameters) {
  while (true) {
    updateMusic();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
