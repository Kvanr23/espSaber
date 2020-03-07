#include <Sound.h>

Sound sound();

void setup() {
  SD.begin(5, SPI, 8000000);
  sound.play("/ON.wav");
}

void loop() {
  
}
