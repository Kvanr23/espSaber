#include <FastLED.h>

#define LEDS_PIN 4
#define NUM_LEDS 59
#define PULSE_SIZE 4

CRGBArray<NUM_LEDS> leds;
CHSV blue = CHSV(155, 255, 255 / 2);
CHSV red = CHSV(0, 255, 255 / 2);
CHSV green = CHSV(100, 255, 255);

CHSV baseColor = green;
CHSV pulseColor = CHSV(39, 255, 255 / 2);

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<NEOPIXEL, LEDS_PIN>(leds, NUM_LEDS);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = baseColor;
    leds(NUM_LEDS, NUM_LEDS - 1) = leds(NUM_LEDS - 1 , 0);
    FastLED.delay(10);
  }
  delay(1000);

}

int r;

void loop() {
  r = random(PULSE_SIZE, NUM_LEDS - PULSE_SIZE);
  Serial.println(r);

  for (int i = -PULSE_SIZE; i < PULSE_SIZE; i++) {
    leds[r + i] = pulseColor;
  }
  FastLED.delay(5);
  for (int i = -PULSE_SIZE; i < PULSE_SIZE; i++) {
    leds[r + i] = baseColor;
  }
  FastLED.delay(10);
}
