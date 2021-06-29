#include <FastLED.h>
#define NUM_LEDS 59
#define PULSE_SIZE 4

CRGBArray<NUM_LEDS> leds;

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<NEOPIXEL, 2>(leds, NUM_LEDS);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(0, 255, 255 / 2);
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
    leds[r + i] = CHSV(26, 255, 255 / 2);
  }
  FastLED.delay(5);
  for (int i = -PULSE_SIZE; i < PULSE_SIZE; i++) {
    leds[r + i] = CHSV(0, 255, 255 / 2);
  }
  FastLED.delay(10);
}
