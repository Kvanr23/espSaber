#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

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

void setup(void) {
  Serial.begin(115200);
  
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  delay(100);
}
long lastSwing = 0;
long timeout = 200;
void loop() {

  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  m = {a.acceleration.x, a.acceleration.y, a.acceleration.z, g.gyro.x, g.gyro.y, g.gyro.z};
  

  /* Print out the values */
  //  Serial.print(a.acceleration.x);
  //  Serial.print(",");
  //  Serial.print(a.acceleration.y);
  //  Serial.print(",");
  //  Serial.print(a.acceleration.z);
  //  Serial.print(", ");
  //  Serial.print(g.gyro.x);
  //  Serial.print(",");
  //  Serial.print(g.gyro.y);
  //  Serial.print(",");
  //  Serial.print(g.gyro.z);
  //  Serial.println("");
  if (millis() >= lastSwing + timeout) {
    if (m.ax >= 20 || m.ay >= 20 || m.az >= 20 || m.ax <= -20 || m.ay <= -20 || m.az <= -20) {
      Serial.println("Swing");
      lastSwing = millis();
    }
  }


  delay(10);
}
