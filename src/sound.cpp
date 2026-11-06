#include "config.h"
#include "sound.h"
#include "sd_card.h"
#include "display.h"
#include "pomodoro.h"
#include <AudioOutputI2S.h>
#include <AudioGeneratorMP3.h>
#include <AudioFileSourceSD.h>

AudioFileSourceSD *fileMusic = nullptr;
AudioFileSourceSD *fileNotif = nullptr;
String notifFilePath = "/notif/harp1.mp3";
AudioGeneratorMP3 *mp3Music = new AudioGeneratorMP3();
AudioGeneratorMP3 *mp3Notif = new AudioGeneratorMP3();
AudioOutputI2S *out = new AudioOutputI2S();


bool isPaused = false;
bool interrupt = false;
bool playNotif = false;
bool isNotif = false;
float volume = 0.02;  // Volume (0.0–1.0)
String currFileName = "";
int bitrateKbps = 320;

unsigned long trackStart = 0;
unsigned long pausedAt = 0;

void initMusic() {
  out->SetPinout(SPEAKER_BCLK, SPEAKER_LRC, SPEAKER_DIN);
  out->SetGain(volume);

  pinMode(SPEAKER_SD, OUTPUT);
  digitalWrite(SPEAKER_SD, LOW);
  digitalWrite(SPEAKER_SD, HIGH);

  playPauseMusic();
}

void updateMusic() {
  if (interrupt) return;

  if (playNotif) {
    playNotif = false;
    endCycleTune();
  }

  if (isNotif) {
    if (!mp3Notif->loop()) {
      mp3Notif->stop();

      delete fileNotif;
      fileNotif = nullptr;

      digitalWrite(SPEAKER_SD, LOW);
      isNotif = false;

      playRandomTrack();
    }
  } else {
    if (!isPaused && mp3Music->isRunning()) {
      if (!mp3Music->loop()) playRandomTrack();
    }
  }
}

void playRandomTrack() {
  int randomIndex;
  String newFileName;

  do {
    randomIndex = random(musicCount);
    newFileName = musicFiles[randomIndex];
  } while (currFileName == newFileName);

  // shift remaining tracks down
  for (int i = randomIndex; i < musicCount - 1; i++) {
    musicFiles[i] = musicFiles[i + 1];
  }
  musicCount--;

  // Reset music queue when each song played at least once
  if (musicCount == 0) {
    loadMusicFiles("/music");
  }

  currFileName = newFileName;
  startTrack("/music/", currFileName);
}

void startTrack(String folder, String filename) {
  if (mp3Music) {
    delete mp3Music;
    mp3Music = nullptr;
  }

  if (out) {
    out->stop();
    delete out;
    out = nullptr;
  }

  out = new AudioOutputI2S();
  out->SetPinout(SPEAKER_BCLK, SPEAKER_LRC, SPEAKER_DIN);
  out->SetGain(volume);

  mp3Music = new AudioGeneratorMP3();

  delete fileMusic;
  fileMusic = nullptr;

  String path = folder + filename;
  fileMusic = new AudioFileSourceSD(path.c_str());

  if (!fileMusic->isOpen()) {
    Serial.println("Failed to open file " + path);
    return;
  }

  mp3Music->begin(fileMusic, out);

  digitalWrite(SPEAKER_SD, HIGH);
  Serial.println("Playing " + path);

  if (isPaused) pausedAt = millis();
  trackStart = millis();
  newTrack(getTrackName(), getTrackMaxS());
}

void playPauseMusic() {
  while (mp3Notif->isRunning()) ;

  isPaused = !isPaused;
  digitalWrite(SPEAKER_SD, isPaused ? LOW : HIGH);
  Serial.println(isPaused ? "Paused" : "Resumed");

  if (isPaused) {
    pausedAt = millis();
  } else {
    trackStart += (millis() - pausedAt);
  }

  togglePlayPauseSymbol();
}

bool isPlaying() {
  return !isPaused;
}

void changeTrack() {
  interrupt = true;
  playRandomTrack();
  
  if (isPaused) playPauseMusic();
  interrupt = false;
}

void changeVol(bool up) {
  if (up) {
    volume += 0.01;
  } else {
    volume -= 0.01;
  }
  
  volume = constrain(volume, 0, 1);
  out->SetGain(volume);
  toggleVolBar();
}

float getVol() {
  return volume*4;
}

String getTrackName() {
  return currFileName.substring(0, currFileName.indexOf('.'));
}

int getTrackCurrS() {
  if (isPaused) return (pausedAt - trackStart) / 1000;
  return (millis() - trackStart) / 1000;
}

int getTrackMaxS() {
  if (!fileMusic->isOpen()) return 0;
  
  uint32_t fileSize = fileMusic->getSize(); // bytes

  // bitrateKbps * 1000 bits per sec, divide by 8 to get bytes per sec
  return fileSize / (bitrateKbps * 1000 / 8);
}

void setPlayNotif() {
  playNotif = true;
}

void endCycleTune() {
  if (mp3Music->isRunning()) mp3Music->stop();

  fileNotif = new AudioFileSourceSD(notifFilePath.c_str());
  if (!fileNotif->isOpen()) {
    Serial.println("Failed to open file " + notifFilePath);
    delete fileNotif;
    fileNotif = nullptr;
    isNotif = false;
    return;
  }

  digitalWrite(SPEAKER_SD, HIGH);
  mp3Notif->begin(fileNotif, out);
  isNotif = true;
  mp3Notif->loop();
}