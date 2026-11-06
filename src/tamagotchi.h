#ifndef TAMAGOTCHI_H
#define TAMAGOTCHI_H

#include <Arduino.h>

enum Pet {
  CAT,
  DOG,
  OWL
};

enum Emotion {
  TEST,
  NEUTRAL,
  HAPPY,
  SAD,
  ANGRY,
  SICK,
  EXCITED,
  FOCUS,
  RELAX,
  SLEEP,
  TIRED
};

struct PetStats {
  float happiness;
  float tiredness;
  float wellness;
};

void initTamagotchi();
void updateTamagotchi(float aqi);
void tamagotchiHappiness(bool happy);
float getTamagotchiHappiness();
bool isTamagotchiHappy();
void setEmotion(Emotion emotion);
String getPetFileName();
String getEmotionFileName();

#endif