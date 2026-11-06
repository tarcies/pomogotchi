#ifndef SD_CARD_H
#define SD_CARD_H

#include "FS.h"

extern String musicFiles[50];
extern int musicCount;

void initSD();
void loadMusicFiles(const char *dirName);
fs::File openFile(const char *filename);
fs::File openFileMode(const char *filename, const char *mode);
bool fileExists(const char *filename);


#endif
