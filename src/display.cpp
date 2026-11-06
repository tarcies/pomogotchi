// 480 x 320
#include "display.h"
#include "sound.h"
#include "sd_card.h"
#include "pomodoro.h"
#include "tamagotchi.h"
#include "tasks.h"
#include "web_server.h"
#include <TFT_eSPI.h>
#include <FS.h>

#define RGB565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

#define PRIMARY RGB565(248, 226, 238)
#define SECONDARY RGB565(3, 62, 64)

#define TIMER_BG_REST RGB565(230, 255, 230)
#define HAPPINESS_BAR_HAPPY RGB565(230, 255, 230) // Green-ish
#define HAPPINESS_BAR_SAD RGB565(255, 235, 235) // Red-ish
#define TRACK_TIMELINE RGB565(3, 32, 34)
#define TASK_CHECK_LINE RGB565(50, 50, 50)

TFT_eSPI tft = TFT_eSPI();
char buf[32];

uint32_t BACKGROUND = RGB565(255, 255, 255);
bool pomodoroLastState = false;

String trackName;
int trackMaxS;
int trackCurrS;
bool musicMode = false;
bool taskMode = false;

int pomoTimerS;
bool pomoFocus;
bool pomoRunning;

int selectedTaskID = 0;
int taskPage;

bool darkMode = false;

void initDisplay() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(BACKGROUND);

  drawBmp(getPetFileName().c_str(), 144, 64, false);
  drawBmp(getEmotionFileName().c_str(), 144, 64, true);
}

void startDisplay() {
  tft.fillScreen(BACKGROUND);

  tft.fillRect(0, 0, 480, 60, PRIMARY);               // Top bar
  tft.fillCircle(290, 30, 17, SECONDARY);             // AQI Border
  tft.fillRect(332, 15, 3, 30, SECONDARY);            // Separator
  tft.fillRoundRect(10, 85, 235, 150, 12, SECONDARY); // Pomodoro timer border
  tft.setTextColor(SECONDARY, isPomodoroFocus ? PRIMARY : TIMER_BG_REST); // Pomodoro bg
  
  updateBotBar();
}

void updateDisplay(float temp, float humid, int aqi, String currTime, int pomodoroTimerS, bool pomodoroFocus, bool pomodoroRunning, int songMaxS, bool dark) {
  if (taskMode) return;

  if (darkMode != dark) {
    darkMode = dark;
    Serial.print("Switch to ");
    Serial.print(darkMode ? "Dark" : "Light");
    Serial.println(" Mode.");

    if (darkMode) {
      BACKGROUND = RGB565(0, 0, 0);
    } else {
      BACKGROUND = RGB565(255, 255, 255);
    }

    startDisplay();
    updatePomodoroTab(true);
  }

  topBar(temp, humid, aqi, currTime);
  pomodoro(pomodoroTimerS, pomodoroFocus, pomodoroRunning);
  updateTrack(songMaxS);
}

void topBar(float temp, float humid, int aqi, String time) {
  tft.setTextColor(SECONDARY, PRIMARY);
  tft.setTextDatum(ML_DATUM);
  tft.setTextSize(3);

  // Temperature
  sprintf(buf, "%.1f%cC", temp, 247);
  tft.drawString(buf, 30, 32);

  // Humidity
  sprintf(buf, "%.1f%%", humid);
  tft.drawString(buf, 160, 32);

  // Air Quality Index
  uint16_t aqiColour;
  switch (aqi) {
    case 1:
      tft.setTextColor(TFT_WHITE);
      aqiColour = RGB565(0, 230, 0);
      break;
    case 2:
      tft.setTextColor(SECONDARY);
      aqiColour = RGB565(147, 255, 40);
      break;
    case 3:
      tft.setTextColor(SECONDARY);
      aqiColour = RGB565(255, 255, 30);
      break;
    case 4:
      tft.setTextColor(TFT_WHITE);
      aqiColour = RGB565(255, 206, 20);
      break;
    case 5:
      tft.setTextColor(TFT_WHITE);
      aqiColour = RGB565(255, 30, 30);
      break;
  }
  tft.fillCircle(290, 30, 15, aqiColour);

  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(String(aqi), 292, 31);

  // Time
  tft.setTextColor(SECONDARY, PRIMARY);
  tft.setTextSize(3);
  tft.setTextDatum(MR_DATUM);
  sprintf(buf, "%s", time);
  tft.drawString(buf, 450, 32);
}


void pomodoro(int timerS, bool isFocus, bool isRunning) {
  pomoTimerS = timerS;
  pomoFocus = isFocus;
  pomoRunning = isRunning;

  updatePomodoroTab(false);
}

void updatePomodoroTab(bool darkMode) {
  tft.setTextColor(SECONDARY, pomoFocus ? PRIMARY : TIMER_BG_REST);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(6);
  sprintf(buf, "%2d:%02d", pomoTimerS / 60, pomoTimerS % 60);

  if (darkMode) {
    tft.fillRoundRect(20, 95, 215, 130, 12, pomoFocus ? PRIMARY : TIMER_BG_REST);
    tft.drawString(buf, 130, 160);
  }

  // End of a phase
  if (pomodoroLastState != pomoFocus) {
    pomodoroLastState = pomoFocus;
    tft.fillRoundRect(20, 95, 215, 130, 12, pomoFocus ? PRIMARY : TIMER_BG_REST);
    tft.drawString(buf, 130, 160);
  }

  if (pomoRunning) {
    tft.drawString(buf, 130, 160);
  }
}

void drawTamagotchi() {
  if (taskMode) return;
  drawBmp(getPetFileName().c_str(), 245, 64, false);
  drawBmp(getEmotionFileName().c_str(), 245, 64, true);

  happinessBar();
}

void drawBmp(const char *filename, int16_t x, int16_t y, bool transparent) {
  File bmpFile;
  int16_t w, h;
  uint8_t r, g, b;
  uint32_t rowSize;
  uint8_t sdbuffer[3 * 20];  // read 20 pixels at a time
  uint8_t buffidx = sizeof(sdbuffer);
  boolean flip = true;
  int pixelSize = 6;

  bmpFile = openFile(filename);
  if (!bmpFile) {
    Serial.println("File not found");
    return;
  }

  // BMP header
  if (read16(bmpFile) == 0x4D42) {
    read32(bmpFile);
    read32(bmpFile);
    uint32_t imageOffset = read32(bmpFile);
    read32(bmpFile);
    w = read32(bmpFile);
    h = read32(bmpFile);
    if (read16(bmpFile) == 1 && read16(bmpFile) == 24 && read32(bmpFile) == 0) {
      rowSize = (w * 3 + 3) & ~3;
      if (h < 0) {
        h = -h;
        flip = false;
      }

      bmpFile.seek(imageOffset);
      for (int row = 0; row < h; row++) {
        uint32_t pos = flip ? imageOffset + (h - 1 - row) * rowSize : imageOffset + row * rowSize;
        if (bmpFile.position() != pos) bmpFile.seek(pos);
        buffidx = sizeof(sdbuffer);
        for (int col = 0; col < w; col++) {
          if (buffidx >= sizeof(sdbuffer)) {
            bmpFile.read(sdbuffer, sizeof(sdbuffer));
            buffidx = 0;
          }
          b = sdbuffer[buffidx++];
          g = sdbuffer[buffidx++];
          r = sdbuffer[buffidx++];
          if (r == 255 && g == 255 && b == 255 && transparent) continue; // Skip transparent (white) pixels
          if (r == 255 && g == 255 && b == 255 && darkMode) {
            r = 0;
            g = 0;
            b = 0;
          }
          tft.fillRect(x + (col * pixelSize), y + (row * pixelSize), pixelSize, pixelSize, RGB565(r, g, b));
        }
      }
    }
  }
  bmpFile.close();
}

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read();
  ((uint8_t *)&result)[1] = f.read();
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read();
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read();
  return result;
}

void happinessBar() {
  tft.fillRect(260, 238, 168, 14, SECONDARY);

  int w = 164;
  int calcW = w * (getTamagotchiHappiness() / 100.0);

  tft.fillRect(262, 240, calcW, 10, isTamagotchiHappy() ? HAPPINESS_BAR_HAPPY : HAPPINESS_BAR_SAD);
}

void botBar() {
  tft.fillRect(0, 260, 480, 60, SECONDARY); // Background

  tft.fillRect(30, 270, 420, 6, TRACK_TIMELINE); // Full track timeline

  tft.setTextColor(PRIMARY, SECONDARY);
  tft.setTextDatum(ML_DATUM);
  tft.setTextSize(1);
  tft.drawString("Now playing...", 30, 285);

  toggleBotHighlight();
  togglePlayPauseSymbol();
}

void updateBotBar() {
  // Refresh bot bar
  botBar();

  // Track name
  tft.setTextColor(PRIMARY, SECONDARY);
  tft.setTextDatum(ML_DATUM);
  tft.setTextSize(2);
  if (trackName.length() > 16) {
    trackName = trackName.substring(0, 16);

  }
  tft.drawString(trackName, 30, 300);
}

void newTrack(String name, int maxS) {
  trackName = name;
  trackMaxS = maxS;

  updateBotBar();
}

void updateTrack(int currS) {
  trackCurrS = currS;

  tft.setTextColor(PRIMARY, SECONDARY);
  tft.setTextDatum(MR_DATUM);
  tft.setTextSize(2);
  sprintf(buf, "%d:%02d / %d:%02d", currS / 60, currS % 60, trackMaxS / 60, trackMaxS % 60);
  tft.drawString(buf, 450, 295);

  tft.fillRect(31, 271, 418 * (float(currS) / float(trackMaxS)), 4, PRIMARY);
}

void togglePlayPauseSymbol() {
  tft.fillRect(284, 284, 20, 20, SECONDARY);

  if (isPlaying()) {
    // Play symbol
    tft.fillTriangle(284, 284, 284, 304, 304, 294, PRIMARY);
  } else {
    // Pause symbol
    tft.fillRect(284, 284, 6, 20, PRIMARY);
    tft.fillRect(298, 284, 6, 20, PRIMARY);
  }
}

void setMusicMode(bool show) {
  musicMode = show ? true : false;
  toggleBotHighlight();
  toggleVolBar();
}

void toggleBotHighlight() {
  tft.fillRect(0, 260, 480, 4, musicMode ? TFT_RED : SECONDARY);
  tft.fillRect(0, 260, 4, 60, musicMode ? TFT_RED : SECONDARY);
  tft.fillRect(0, 316, 480, 4, musicMode ? TFT_RED : SECONDARY);
  tft.fillRect(476, 260, 4, 60, musicMode ? TFT_RED : SECONDARY);
}

void toggleVolBar() {
  tft.fillRect(438, 83, 34, 154, SECONDARY);  // Border

  int h = 150;
  int y = 85;

  int calcH = 150 * getVol();
  int calcY = h - calcH + y;

  tft.fillRect(440, calcY, 30, calcH, PRIMARY);
  
  tft.setTextColor(SECONDARY);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.drawString("Vol", 456, 75);

  if (!musicMode) tft.fillRect(438, 63, 34, 174, BACKGROUND);  // Overlay volume bar to hide it
}

void setTaskMode(bool show) {
  taskMode = show;
  if (taskMode) startTaskTab();
  if (!taskMode) {
    startDisplay();
    updatePomodoroTab(false);
  }
}

void startTaskTab() {
  tft.fillScreen(BACKGROUND);

  tft.setTextColor(SECONDARY);
  tft.setTextDatum(TC_DATUM);
  tft.setTextSize(4);
  tft.drawString("Task List", 240, 20);

  updateTaskTab(false);
}

void updateTaskTab(bool newPage) {
  int x = 20;
  int offsetY = 60;
  int gapY = 20;
  int w = 440;
  int h = 40;

  taskPage = selectedTaskID / 4;
  
  tft.setTextColor(SECONDARY);
  tft.setTextDatum(ML_DATUM);
  tft.setTextSize(3);

  if (newPage) tft.fillRect(0, 70, 480, 320, BACKGROUND);

  for (int i = 0; i < (taskCount - (taskPage*4)); i++) {
    tft.fillRoundRect(x - 4, offsetY + gapY + (i * (gapY + h)) - 4, w + 8, h + 8, 8, (i == selectedTaskID % 4) ? SECONDARY : BACKGROUND);

    tft.fillRoundRect(x, offsetY + gapY + (i * (gapY + h)), w, h, 8, PRIMARY);
    tft.drawString(tasks[(taskPage*4) + i].name, x + 15, offsetY + gapY + (i * (gapY + h)) + h/2);
    Serial.println(tasks[(taskPage*4) + i].name);

    if (tasks[(taskPage*4) + i].done) tft.drawLine(x + 10, offsetY + gapY + (i * (gapY + h)) + h/2, w - 10, offsetY + gapY + (i * (gapY + h)) + h/2, TASK_CHECK_LINE);
  }
}

void changeSelectedTask(bool up) {
  selectedTaskID = selectedTaskID + (up ? 1 : -1);
  updateTaskTab(selectedTaskID % 4 == 0 || selectedTaskID % 4 == 3);
}

void checkTask() {
  tasks[selectedTaskID].done = !tasks[selectedTaskID].done;
  Serial.println("Task ID: " + String(selectedTaskID) + " set to: " + String(tasks[selectedTaskID].done));
  updateTaskTab(false);
  saveTasksToSD();
}
