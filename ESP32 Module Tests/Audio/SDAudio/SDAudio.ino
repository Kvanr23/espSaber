#include "Sound.h"
#include <SD.h>

#define SD_PIN 5
#define saberBtn 32

volatile bool saberState = false;
volatile bool buttonIsPressed = false;

void setup() {
  Serial.begin(115200);
  if (!SD.begin(SD_PIN, SPI, 8000000)) {
    Serial.println("SD Init Failed");
    return;
  }
  pinMode(saberBtn, INPUT);
  setupSound();
}

void loop() {
  audioLoop(0);
  buttonFunction();
}

void buttonFunction(void)
{
  if (debounce(saberBtn, 20) && !soundEffectPlaying())
  {
    Serial.println("Button is pressed");
    if (saberState)
    {
      buttonIsPressed = true;
      playSound("/OFF.wav");
      saberState = false;
    }
    else if (!saberState)
    {
      buttonIsPressed = true;
      playSound("/ON.wav");

      saberState = true;
    }
  }
}

/**
   Function to debouce the button, to prevent the button from false triggering
*/
boolean debounce(int pin, int debounceDelay)
{
  boolean state;
  boolean previousState;

  previousState = digitalRead(pin); // store switch state
  for (int counter = 0; counter < debounceDelay; counter++)
  {
    delay(1);                 // wait for 1 millisecond
    state = digitalRead(pin); // read the pin
    if (state != previousState)
    {
      counter = 0;           // reset the counter if the state changes
      previousState = state; // and save the current state
    }
  }
  return state;
}
