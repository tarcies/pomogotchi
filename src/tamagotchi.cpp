#include "config.h"
#include "pomodoro.h"
#include "tamagotchi.h"
#include "sd_card.h"
#include "display.h"
#include <FS.h>

#define TRANSITION_DURATION_MS 4000

Pet currPet;
Emotion currEmotion;
String petPath = "/tamagotchi/base/";
String emotionPath = "/tamagotchi/emotion/";
bool flip;
unsigned long transitionEmStart = 0;
PetStats pet;

void initTamagotchi() {
  currPet = CAT;
  currEmotion = NEUTRAL;
  flip = false;

  pet = {30, 50, 45};
}

void updateTamagotchi(float aqi) {
  if ((millis() - transitionEmStart < TRANSITION_DURATION_MS) && (currEmotion == ANGRY || currEmotion == RELAX)) return;
  
  // 1. Tiredness (+ from working, - from resting)
  if (isPomodoroRunning() && isPomodoroFocus()) pet.tiredness += 0.02;
  else if (isPomodoroRunning() && !isPomodoroFocus()) pet.tiredness -= 0.1;
  
  // 2. Happiness decreases if idle
  if (!isPomodoroRunning()) pet.happiness -= 0.1;

  // 3. Wellness from air quality
  if (aqi <= 2) pet.wellness += 0.1;
  else if (aqi >= 4) pet.wellness -= 0.3;

  // Clamp values 0–100
  pet.tiredness = constrain(pet.tiredness, 0, 100);
  pet.happiness = constrain(pet.happiness, 0, 100);
  pet.wellness = constrain(pet.wellness, 0, 100);

  if (!isPomodoroRunning()) {
    if (pet.wellness < 40) currEmotion = SICK;
    if (pet.tiredness > 70) currEmotion = TIRED;
    if (pet.happiness < 20) currEmotion = SAD;
  }
}

void tamagotchiHappiness(bool happy) {
  pet.happiness = pet.happiness + (happy ? 15 : (-5));
  pet.happiness = constrain(pet.happiness, 0, 100);
  if (currEmotion != ANGRY || currEmotion != EXCITED) currEmotion = isTamagotchiHappy ? HAPPY : SAD;
}

float getTamagotchiHappiness() {
  return pet.happiness;
}

bool isTamagotchiHappy() {
  return pet.happiness > 30;
}

void setEmotion(Emotion emotion) {
  currEmotion = emotion;
  if (currEmotion == ANGRY || currEmotion == RELAX) transitionEmStart = millis();
}

String getPetFileName() {
  String petName;

  switch (currPet) {
    case CAT:
      petName = "cat";
      break;
    case DOG:
      petName = "dog";
      break;
    case OWL:
      petName = "owl";
      break;
  }

  String filename;
  filename = petPath + petName + ".bmp";
  return filename;
}

String getEmotionFileName() {
  String emotion;
  flip = !flip;

  switch (currEmotion) {
    case TEST:
      emotion = "test";
      break;
    case NEUTRAL:
      emotion = "neutral";
      break;
    case HAPPY:
      emotion = "happy";
      break;
    case SAD:
      emotion = "sad";
      break;
    case ANGRY:
      emotion = "angry";
      break;
    case SICK:
      emotion = "sick";
      break;
    case EXCITED:
      emotion = "excited";
      break;
    case FOCUS:
      emotion = "focus";
      break;
    case RELAX:
      emotion = "relax";
      break;
    case SLEEP:
      emotion = "sleep";
      break;
    case TIRED:
      emotion = "tired";
      break;
  }

  String filename;
  filename = emotionPath + emotion + (flip ? 1 : 2) + ".bmp";
  return filename;
}