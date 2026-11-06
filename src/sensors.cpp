#include "esp32-hal.h"
#include <stdint.h>
#include "config.h"
#include "sensors.h"
#include "wifi_util.h"
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_BME280.h>
#include <ScioSense_ENS160.h>
#include <RV3028C7.h>

Adafruit_BME280 bme;
ScioSense_ENS160 ens160(ADDR_ENS160);
RV3028C7 rtc;

void initSensors() {
  Wire.begin(I2C_SDA, I2C_SCL);

  if (!bme.begin(ADDR_BME280)) {
    Serial.println("BME280 not found!");
  }

  if (!ens160.begin()) {
    Serial.println("ENS160 not found!");
  } else {
    Serial.println("ENS160 detected. Initialising...");
    ens160.setMode(ENS160_OPMODE_STD);
    delay(2000); // warm-up
    Serial.println("ENS160 ready.");
  }

  if (!rtc.begin()) {
    Serial.println("RTC not found!");
  }

  if (WiFi.status() == WL_CONNECTED) {
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char isoTime[25];
      strftime(isoTime, sizeof(isoTime), "%Y-%m-%dT%H:%M:%S", &timeinfo);
      rtc.setDateTimeFromISO8601(isoTime);
      rtc.synchronize();
      Serial.print("RTC synced with NTP: ");
      Serial.println(isoTime);
      Serial.println("RTC synced with NTP.");
    } else {
      Serial.println("NTP sync failed.");
    }
  } else {
    Serial.println("WiFi not connected.");
  }

  pinMode(PHOTORES_PIN, INPUT);
}

float getTemperature() {
  return bme.readTemperature();
}

float getHumidity() {
  return bme.readHumidity();
}

int getAirQuality() {
  if (!ens160.available()) return -1;
  ens160.measure();
  return ens160.getAQI();
}

bool getDark() {
  return (!digitalRead(PHOTORES_PIN));
}

String getTime() {
  String time = String(rtc.getCurrentDateTime());
  time = time.substring(11, 16);
  return time;
}

String getDateTime() {
  return String(rtc.getCurrentDateTime());
}