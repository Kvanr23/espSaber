#include <FastLED.h>
#define saberBtn 32

#define NUM_LEDS 60
#define STRIP_TYPE WS2812B
#define RGB_ORDER GRB
#define DATA_PIN 2

CRGB leds[NUM_LEDS];

volatile bool saberState = false;
volatile bool buttonIsPressed = false;
volatile bool interrupted = false;

void IRAM_ATTR ISR() {
    interrupted = true;
}

void setup() {
  Serial.begin(115200);

  pinMode(saberBtn, INPUT);

  FastLED.addLeds<STRIP_TYPE, DATA_PIN, RGB_ORDER>(leds, NUM_LEDS);
//  turnSaber(0);
  attachInterrupt(saberBtn, ISR, RISING);
}

int brightness1 = 250;
int brightness2 = 200;
uint8_t hue = 0;

void loop() {
  buttonFunction();

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
    
  }
//  hue++;
//  Serial.println(hue);
}

void buttonFunction(void)
{
  if (debounce(saberBtn, 20))
  {
    Serial.println("Button is pressed");
    if (saberState)
    {
      buttonIsPressed = true;
      saberState = false;
    }
    else if (!saberState)
    {
      buttonIsPressed = true;
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
  Serial.print("Turnsaber: ");
  Serial.println(state);
  if (state)
  {
    int brightness = 1;
    for (int j = 0; j < 256; j=j+50) {
      Serial.println(j);
      for (int i = 0; i < NUM_LEDS; i++)
      {
        leds[i].r = 255;
  //      leds[i].setRGB(255, 0, 0);
//        leds[i] = CHSV(hue, 255, 255);
        FastLED.show();
//        FastLED.delay(1);
      }
//      delay(1);
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
