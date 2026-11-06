// config.h
#ifndef CONFIG_H
#define CONFIG_H

// ------------------ WiFi ------------------
#define WIFI_SSID "WiFi-8EED"
#define WIFI_PASS "54608399"

// ------------------ NTP ------------------
extern const char* NTP_SERVER;
extern const long  GMT_OFFSET_SEC;
extern const int   DAYLIGHT_OFFSET_SEC;

// ------------------ POMODORO ------------------
#define FOCUS_PERIOD_M 25
#define REST_PERIOD_M 5
#define LONG_REST_PERIOD_M 20

// ------------------ I2C CONFIG ------------------
#define I2C_SDA 8
#define I2C_SCL 9

// I2C Addresses for PiicoDev modules
#define ADDR_RV3028  0x52
#define ADDR_ENS160  0x53
#define ADDR_BME280  0x77

// ------------------ PHOTORESISTOR ------------------
#define PHOTORES_PIN 0
#define PHOTORES_THRESHOLD 400

// ------------------ SPEAKER ------------------
#define SPEAKER_SD   39
#define SPEAKER_DIN  40
#define SPEAKER_BCLK 41
#define SPEAKER_LRC  42

// ------------------ SD CARD ------------------
#define SD_SCK  36
#define SD_MISO 37
#define SD_MOSI 35
#define SD_CS   34

// ------------------ BUTTONS ------------------
#define BTN_PIN_L 38
#define BTN_PIN_M 44
#define BTN_PIN_R 43

// ------------------ LCD DISPLAY ------------------
#define LCD_RST 16
#define LCD_CS  15
#define LCD_RS  14
#define LCD_WR  13
#define LCD_RD  12

// 8-bit parallel line
#define LCD_D0  21
#define LCD_D1  1
#define LCD_D2  2
#define LCD_D3  3
#define LCD_D4  4
#define LCD_D5  5
#define LCD_D6  6
#define LCD_D7  7
#define LCD_D8  8

#define DEBUG true

#endif
