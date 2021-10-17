//#include "Sound.h"
#include <SD.h>

#define SD_PIN 5

long startTime;

void setup() {
  Serial.println(115200);
  if(!SD.begin(SD_PIN, SPI, 8000000)) {
    Serial.println("SD Init Failed");
    return;
  }
  
//  setupSound();

  startTime = millis();
}

void loop() {
//  audioLoop(0);
//  long roundtime = millis();
//
//  if ((roundtime - startTime) == 1000) {
//    playSound("/ON.wav");
//  }
}
