// Includes
#include <FastLED.h>
#include <SD.h>
#include "Sound.h"

// Defines
#define SD_PIN 5
#define saberBtn 32

#define NUM_LEDS 60
#define STRIP_TYPE WS2812B
#define RGB_ORDER GRB
#define DATA_PIN 2

// Variables
CRGB leds[NUM_LEDS];
int brightness1 = 250;
int brightness2 = 200;
uint8_t hue = 0;

volatile bool saberState = false;
volatile bool buttonIsPressed = false;
volatile bool interrupted = false;

// Dual core task
TaskHandle_t Task1;

// Interrupt method
void IRAM_ATTR ISR() {
  interrupted = true;
}

void setup()
{
  // Initialize Serial
  Serial.begin(115200);
  if (!SD.begin(SD_PIN, SPI, 8000000)) {
    Serial.println("SD Init Failed");
    return;
  }
  // Set pinmode of button
  pinMode(saberBtn, INPUT);
  // Attach interrupt to button
  attachInterrupt(saberBtn, ISR, RISING);
  // Setup sound
  setupSound();
  // Setup ledstrip
  FastLED.addLeds<STRIP_TYPE, DATA_PIN, RGB_ORDER>(leds, NUM_LEDS);
  // Setup second core
  xTaskCreatePinnedToCore(
    loop2,
    "Task1",
    10000,
    NULL,
    0,
    &Task1,
    0
  );
}

void loop()
{
  audioLoop(0);
  buttonFunction();
}

void loop2(void * parameters) {
  for (;;) {
    if (buttonIsPressed) {
      Serial.println("BUTTON");
      if (saberState) {
        turnSaber(1);
        buttonIsPressed = false;
      }
      else if (!saberState) {
        turnSaber(0);
        buttonIsPressed = false;
      }
    }

    if (saberState) {
      kyloPulse();
    }
  }
}

void buttonFunction(void)
{
  if (debounce(saberBtn, 20) && !soundEffectPlaying())
  {
    Serial.println("Button is pressed");
    if (saberState)
    {
      buttonIsPressed = true;
      saberState = false;
      playSound("/OFF.wav");
    }
    else if (!saberState)
    {
      buttonIsPressed = true;
      saberState = true;
      playSound("/ON.wav");
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
  Serial.print("Turnsaber: ");
  Serial.println(state);
  if (state)
  {
    int brightness = 1;
    for (int j = 50; j < 256; j = j + 50) {
      Serial.println(j);
      for (int i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = CHSV(hue, 255, j);
        FastLED.show();
      }
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

void kyloPulse() {
  for (int i = 0; i < NUM_LEDS; i++)
  {
    if (interrupted) {
      interrupted = false;
      break;
    }
    leds[i] = CHSV(hue, 255, brightness1);
  }
  for (int i = 0; i < NUM_LEDS; i++)
  {
    if (interrupted) {
      interrupted = false;
      break;
    }
    leds[i] = CHSV(hue, 255, brightness2);
    FastLED.show();
  }
}
