#include "Settings.h"

#include "Sound.h"

Sound sound(DAC0);

void setup() {
  SD.begin(5, SPI, 8000000);
  sound.play("/ON.wav");
}

void loop() {
  sound.audioLoop(1);
}
