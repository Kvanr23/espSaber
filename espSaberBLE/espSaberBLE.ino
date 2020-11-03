/**
   Includes
*/

// Bluetooth
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// FastLED
#include <FastLED.h>

// Sound
#include "Sound.h"


/**
   Defines
*/

// Bluetooth Low Energy
#define BT_NAME             "ESP Saber"

#define SERVICE_UUID        "65e90000-4abe-43bf-87c4-cacbae83c5c1"
#define COLOR_UUID          "65e90001-4abe-43bf-87c4-cacbae83c5c1"
#define TYPE_UUID           "65e90002-4abe-43bf-87c4-cacbae83c5c1"

// Button
#define saberBtn            32

// FastLED
#define NUM_LEDS            60
#define STRIP_TYPE          WS2812B
#define RGB_ORDER           GRB
#define DATA_PIN            2


/**
   Variables & constants
*/

// Saber Settings     R     G     B
byte saberColor[3] = {0x00, 0x00, 0xFF};
CRGB color = CRGB(255, 0, 0);
char saberType = 0x01;
CRGB colorDef = CRGB::Red;

// Ledstrip
CRGB leds[NUM_LEDS];

// Dual core
TaskHandle_t Task1;

// Saber values
volatile bool saberState = false;
volatile bool buttonIsPressed = false;


/**
   Callbacks for BLE Characteristics
*/

// Saber Color Characteristic
class ColorCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        color = CRGB(value[0],value[1], value[2]);
        for (int i = 0; i < 3; i++) {
          saberColor[i] = value[i];
        }
      }
    }
};

// Saber Type Characteristic
class TypeCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        switch (value[0]) {
          case 0x01:
            Serial.println("Regular Mode");
            saberType = value[0];
            break;
          case 0x02:
            Serial.println("Kylo Ren Mode");
            saberType = value[0];
            break;
          case 0x03:
            Serial.println("Darth Maul Mode");
            saberType = value[0];
            break;
          default:
            saberType = 0x01;
            break;
        }
      }
    }
};


/**
   BLE Functions
*/

// Setup BLE Service and Characteristics
int setupBLE() {
  // Startup the BLE code and set the name of the device/
  BLEDevice::init(BT_NAME);

  // Start the BLE server
  BLEServer *pServer = BLEDevice::createServer();

  // Create the a service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create the characteristics inside the service.
  /* Color */
  BLECharacteristic *pColorChar = pService->createCharacteristic(
                                    COLOR_UUID,
                                    BLECharacteristic::PROPERTY_READ |
                                    BLECharacteristic::PROPERTY_WRITE
                                  );

  /* Type */
  BLECharacteristic *pTypeChar = pService->createCharacteristic(
                                   TYPE_UUID,
                                   BLECharacteristic::PROPERTY_READ |
                                   BLECharacteristic::PROPERTY_WRITE
                                 );

  // Set the callbacks for the characteristics
  pColorChar->setCallbacks(new ColorCallbacks());
  pTypeChar->setCallbacks(new TypeCallbacks());

  // Set start values.
  //  pColorChar->setValue(saberColor);
  //  pTypeChar->setValue(saberType);

  // TODO: ^ Disabled for now, need to find a way to save a char array.

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

/**
   Setup
*/
void setup()
{
  // Start Serial
  Serial.begin(9600);
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

  // Setup BLE
  setupBLE();

  // Start task on second core
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

/**
   Loop
*/
void loop()
{
  audioLoop(1);
  buttonFunction();
  delay(1);
}

/**
   Loop for lighting
*/
void lightingLoop(void * parameters) {
  for (;;) {
//    bool state = saberState;
//    bool button = buttonIsPressed;
//
//    if (button) {
//      Serial.println("A");
//    }
//    
    if (buttonIsPressed) {
      if (saberState) {
        Serial.println("Saber off");
        turnSaber(0);
        buttonIsPressed = false;
      }
      else if (!saberState) {
        Serial.println("Saber on");
        turnSaber(1);
        buttonIsPressed = false;
      }
    }
  }
}

/**
   Button logics
*/
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

/**
   Function to change the saber state
*/
void turnSaber(bool state) {
  Serial.println("turnSaber");
  if (state)
  {
    for (int i = 0; i < NUM_LEDS; i++)
    {
      // Use color we defined at the top, which can be changed with BLE.
      leds[i] = color;
      FastLED.show();
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
