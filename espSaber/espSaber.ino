#include <WiFi.h>
#include <FastLED.h>

#include "Sound.h"

#define saberBtn 32

#define NUM_LEDS 60
#define STRIP_TYPE WS2812B
#define RGB_ORDER GRB
#define DATA_PIN 2

CRGB leds[NUM_LEDS];

// Dual core
TaskHandle_t Task1;

long period = 250;

void setup()
{
  // Start Serial
  Serial.begin(115200);
  // Start SD connection
  if (!SD.begin(5, SPI, 8000000))
  {
    Serial.println("SD Failed");
    return;
  }

  // Add Strip
  FastLED.addLeds<STRIP_TYPE, DATA_PIN, RGB_ORDER>(leds, NUM_LEDS);
  // Turn off all leds right away
  turnSaber(0);

  delay(100);
  // Setup sound
  setupSound();
  delay(100);

  // Pinmodes
  pinMode(saberBtn, INPUT_PULLDOWN);

  xTaskCreatePinnedToCore(
    lightingLoop,
    "Task1",
    10000,
    NULL,
    0,
    &Task1,
    0
  );
}

bool saberState = false;

void loop()
{
  audioLoop(1);
  buttonFunction();
  delay(1);
}

bool buttonIsPressed = false;

void lightingLoop(void * parameters) {
  for (;;) {
    Serial.println("TEST");
    if (buttonIsPressed) {
      Serial.println("BUTTON");
      if (saberState) {
        Serial.println("SABER ON");
        turnSaber(0);
        buttonIsPressed = false;
      }
      else if (!saberState) {
        turnSaber(1);
        buttonIsPressed = false;
      }
    }
  }
}

void buttonFunction(void)
{
  if (debounce(saberBtn, 20) && !soundEffectPlaying())
  {
    if (saberState)
    {
      buttonIsPressed = true;
//      turnSaber(0);
      playSound("/OFF.wav");
      saberState = false;
    }
    else if (!saberState)
    {
      buttonIsPressed = true;
//      turnSaber(1);
      playSound("/ON.wav");

      saberState = true;
    }
  }
}

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

void turnSaber(bool state) {
  if (state)
  {
    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i].r = 255;
      FastLED.show();
//      FastLED.delay(1);0
    }
  }
  else if (!state)
  {
    for (int i = NUM_LEDS - 1; i >= 0; i--)
    {
      leds[i] = CRGB::Black;
      FastLED.show();
      FastLED.delay(1);
    }
  }
}
