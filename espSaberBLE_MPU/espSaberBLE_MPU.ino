/**
   Includes
*/
// EEPROM
#include <EEPROM.h>
// ESP32 ADC
#include <driver/adc.h>
// Bluetooth
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
// FastLED
#include <FastLED.h>
// Sound
#include <SD.h>
#include "Sound.h"
// Motion
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

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

// Batterylevel
#define batteryPin          34

// FastLED
#define NUM_LEDS            60
#define STRIP_TYPE          WS2812B
#define RGB_ORDER           GRB
#define DATA_PIN            2

/**
   Variables & constants
*/
// Motion
Adafruit_MPU6050 mpu;
struct Motion {
  int ax;
  int ay;
  int az;
  int gx;
  int gy;
  int gz;
};
Motion m = {0, 0, 0, 0, 0, 0};
long lastSwing = 0;
long timeout = 200;

// Saber Settings
byte saberColor[3] = {0x00, 0x00, 0xFF};
CRGB color = CRGB(255, 0, 0);
CHSV colorHue = rgb2hsv_approximate(color);
char saberType = 0x01;

// Bluetooth Low Energy
BLEServer* pServer = NULL;
BLECharacteristic *pColorChar = NULL;
BLECharacteristic *pTypeChar = NULL;
BLECharacteristic *pBattChar = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Battery
uint8_t battery_level = 57;
int batterySum = 0;
int batteryAvg = 0;
int batteryMeasurement = 0;

// Ledstrip
CRGB leds[NUM_LEDS];
int brightness1 = 250;
int brightness2 = 200;
uint8_t hue = 0;

// Sound
char *SwingSounds[] = {
  "/SWL1.wav",
  "/SWL2.wav",
  "/SWL3.wav",
  "/SWL4.wav",
  "/SWS1.wav",
  "/SWS2.wav",
  "/SWS3.wav",
  "/SWS4.wav",
  "/SWS5.wav"
};

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
   EEPROM
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
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

// Saber Color Characteristic
class ColorCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      Serial.println("Color update!");
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        color = CRGB(value[0], value[1], value[2]);
        colorHue = rgb2hsv_approximate(color);
        for (int i = 0; i < 3; i++) {
          if (value[i] == 0x00) {
            value[i] = 0x01;
          } else {
            saberColor[i] = value[i];
          }
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
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

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
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  BLEDevice::startAdvertising();
  Serial.println("Started advertising");
}


/**
   Function to update the value of the characteristics
   CRGB value is saved backwards, so we read them in correctly as for updating.
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
   Check Battery
*/
void checkBattery() {
  int measure = adc1_get_raw(ADC1_CHANNEL_6);
  float voltage = (1.5/4095.0) * measure * 8.5;
  int millivolts = voltage * 1000;
  int constrained = constrain(millivolts, 5600, 8400);
  int percentage = map(constrained, 6000, 8400, 0, 100);  
  
  if (batteryAvg == 0) {
    batteryAvg = percentage;
    batterySum = percentage;
  } else {
    batterySum += percentage;
    batteryAvg = batterySum / 2;
    batterySum = batteryAvg;
  }
  
  battery_level = batteryAvg;
  if (battery_level < 10) {
    Serial.println("Battery Low!");
  }
}

/**
   Setup Motion
*/
void setupMotion() {
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
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

  // Init Motion
  setupMotion();
  // Set pinmode of batterylevel
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_0);

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
long elapsed = 0;
int interval = 5000;
void loop()
{
  audioLoop(0);
  buttonFunction();
  if (millis() > (elapsed + interval)) {
    checkBattery();
    elapsed = millis();
  }
  updateCharacteristics();
  swingTick();
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
   Swing Tick
*/
bool swingTick() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  m = {a.acceleration.x, a.acceleration.y, a.acceleration.z, g.gyro.x, g.gyro.y, g.gyro.z};
  if (millis() >= lastSwing + timeout && !saberState && !soundEffectPlaying()) {
    //    Serial.println("Swing tick");
    if (m.ax >= 20 || m.ay >= 20 || m.az >= 20 || m.ax <= -20 || m.ay <= -20 || m.az <= -20) {
      playSound(SwingSounds[random(0, 8)]);
      lastSwing = millis();
    }
  }
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
    Serial.print("Hue: ");
    Serial.println(colorHue.h);
    for (int j = 1; j < 256; j = j + 75) {
      for (int i = 0; i < NUM_LEDS; i++)
      {
        //        leds[i] = color;
        colorHue = rgb2hsv_approximate(color);
        colorHue.v = j;
        leds[i] = colorHue;

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
    //    leds[i] = CHSV(hue, 255, brightness1);
    colorHue = rgb2hsv_approximate(color);
    colorHue.v = brightness1;
    leds[i] = colorHue;
  }
  for (int i = 0; i < NUM_LEDS; i++)
  {
    if (interrupted) {
      interrupted = false;
      break;
    }
    //    leds[i] = CHSV(hue, 255, brightness2);
    colorHue = rgb2hsv_approximate(color);
    colorHue.v = brightness2;
    leds[i] = colorHue;

    FastLED.show();
  }
}
