/**
   Includes
*/
// EEPROM
#include <EEPROM.h>
// Bluetooth
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
// FastLED
#include <FastLED.h>
// Sound
#include <SD.h>
#include "Sound.h"

/**
   Defines
*/
// EEPROM                   Amount of bytes
#define EEPROM_SIZE         4
// Bluetooth Low Energy
#define BT_NAME             "ESP Saber"
#define SERVICE_UUID        "65e90000-4abe-43bf-87c4-cacbae83c5c1"
#define COLOR_UUID          "65e90001-4abe-43bf-87c4-cacbae83c5c1"
#define TYPE_UUID           "65e90002-4abe-43bf-87c4-cacbae83c5c1"
#define BatteryService BLEUUID((uint16_t)0x180F)
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
// Saber Settings
byte saberColor[3] = {0x00, 0x00, 0xFF};
CRGB color = CRGB(255, 0, 0);
char saberType = 0x01;

BLECharacteristic *pColorChar;
BLECharacteristic *pTypeChar;
BLECharacteristic *pBattChar;

// Battery
uint8_t battery_level = 57;

// Ledstrip
CRGB leds[NUM_LEDS];
int brightness1 = 250;
int brightness2 = 200;
uint8_t hue = 0;

// Dual core
TaskHandle_t Task1;

// Saber values
volatile bool saberState = false;
volatile bool buttonIsPressed = false;
volatile bool interrupted = false;

// Interrupt method
void IRAM_ATTR ISR() {
  interrupted = true;
}

/**
 * EEPROM
 */
void saveEEPROM() {
  Serial.println("Saving settings");

  Serial.print("R:");
  Serial.print(color.r);
  Serial.print(" G:");
  Serial.print(color.g);
  Serial.print(" B:");
  Serial.println(color.b);
  
  EEPROM.write(0, color.r);
  EEPROM.write(1, color.g);
  EEPROM.write(2, color.b);
  EEPROM.write(3, saberType);
  EEPROM.commit();
}

void loadEEPROM() {
  Serial.println("Loading Settings");
  uint8_t r = EEPROM.read(0);
  uint8_t g = EEPROM.read(1);
  uint8_t b = EEPROM.read(2);
  Serial.print("R:");
  Serial.print(r);
  Serial.print(" G:");
  Serial.print(g);
  Serial.print(" B:");
  Serial.println(b);
  color.setRGB(r, g, b);
  
  saberType = EEPROM.read(3);
}

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
        saveEEPROM();
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
        saveEEPROM();
      }
    }
};


/**
   BLE Functions
*/
// Setup BLE Service and Characteristics
int setupBLE() {
  Serial.println("BLE init");
  // Startup the BLE code and set the name of the device/
  BLEDevice::init(BT_NAME);

  // Start the BLE server
  BLEServer *pServer = BLEDevice::createServer();

  // Create the services
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLEService *pBattery = pServer->createService(BatteryService);
  
  // Create the characteristics inside the service.
  /* Color */
  /*BLECharacteristic* */
  pColorChar = pService->createCharacteristic(
    COLOR_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );

  /* Type */
  /*BLECharacteristic* */
  pTypeChar = pService->createCharacteristic(
    TYPE_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );

  // Battery level
  /*BLECharacteristic* */
  pBattChar = pBattery->createCharacteristic(
    BLEUUID((uint16_t)0x2A19), 
    BLECharacteristic::PROPERTY_READ | 
    BLECharacteristic::PROPERTY_NOTIFY
  );
  

  pBattChar->addDescriptor(new BLE2902());
  
  // Set the callbacks for the characteristics
  pColorChar->setCallbacks(new ColorCallbacks());
  pTypeChar->setCallbacks(new TypeCallbacks());

  // Set start values.
  pColorChar->setValue(saberColor, 3);
  //  pTypeChar->setValue(saberType);
  // TODO: ^ Disabled for now, need to find a way to save a char array.

  // Battery level set value for now.
  pBattChar->setValue(&battery_level, 1);
  
  // Start the services
  pService->start();
  pBattery->start();

  // Start advertising
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}


/**
 * Function to update the value of the characteristics
 * CRGB value is saved backwards, so we read them in correctly as for updating.
 */
void updateCharacteristics() {
  uint8_t buffer[3];
  buffer[0] = saberColor[2];
  buffer[1] = saberColor[1];
  buffer[2] = saberColor[0];
  
  pColorChar->setValue(buffer, 3);
  pBattChar->setValue(&battery_level, 1);
}

/**
   Setup
*/
void setup()
{
  // Start Serial
  Serial.begin(115200);
  // Start SD connection
  if (!SD.begin(5, SPI, 8000000)) {
    Serial.println("SD Failed");
    return;
  }

  // Init EEPROM
  EEPROM.begin(EEPROM_SIZE);
  loadEEPROM();
  
//  Serial.print("Is EEPROM used before: ");
//  Serial.println(EEPROM.read(4));

  // Set pinmode of button
  pinMode(saberBtn, INPUT);
  // Attach interrupt to button
  attachInterrupt(saberBtn, ISR, RISING);
  // Setup sound
  setupSound();
  // Add Strip
  FastLED.addLeds<STRIP_TYPE, DATA_PIN, RGB_ORDER>(leds, NUM_LEDS);
  // Turn off all leds right away
  turnSaber(0);
  // Setup BLE
  setupBLE();

  // Start task on second core
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

/**
   Loop
*/
void loop()
{
  audioLoop(0);
  buttonFunction();
  updateCharacteristics();
}

/**
   Loop for lighting
*/
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
//    for (int i = 0; i < NUM_LEDS; i++)
//    {
//      // Use color we defined at the top, which can be changed with BLE.
//      leds[i] = color;
//      FastLED.show();
//    }
    int brightness = 1;
    for (int j = 50; j < 256; j = j + 50) {
      Serial.println(j);
      for (int i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = CHSV(hue, 255, j); // TODO: turn RGB into HSV from ble
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
