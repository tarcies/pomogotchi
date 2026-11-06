#include "config.h"
#include "sd_card.h"
#include <FS.h>
#include <SD.h>
#include <SPI.h>

String musicFiles[50];
int musicCount = 0;

void initSD() {
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("SD card failed");
    return;
  }
  Serial.println("SD card ready");
  loadMusicFiles("/music");
}

void loadMusicFiles(const char *dirName) {
  File dir = SD.open(dirName);
  if (!dir || !dir.isDirectory()) return;

  File file = dir.openNextFile();
  while (file && musicCount < 50) {
    if (!file.isDirectory()) {
      String name = file.name();
      // name.toLowerCase();
      if (name.endsWith(".mp3") || name.endsWith(".wav")) {
        musicFiles[musicCount++] = name;
      }
    }
    file = dir.openNextFile();
  }
  dir.close();
}

fs::File openFile(const char *filename) {
  return SD.open(filename);
}

fs::File openFileMode(const char *filename, const char *mode) {
  return SD.open(filename, mode);
}

bool fileExists(const char *filename) {
  return SD.exists(filename);
}
