#ifndef SOUND_H
#define SOUND_H

#include "sd_card.h"

void initMusic();
void updateMusic();
void playRandomTrack();
void startTrack(String folder, String filename);
void playEndPhase();
void playPauseMusic();
bool isPlaying();
void changeTrack();
void changeVol(bool up);
float getVol();
String getTrackName();
int getTrackCurrS();
int getTrackMaxS();
void setPlayNotif();
void endCycleTune();

#endif
