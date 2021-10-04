#include<Wire.h>

int led1G = 9;
int led1R = 10;
int led1B = 6;
int led2R = 5;
int led2G = 4;
int led2B = 3;

const int MPU1 = 0x68;
int16_t AcX1, AcY1, AcZ1, Tmp1, GyX1, GyY1, GyZ1;
byte acXout[3] = {0, 0, 255};
byte acYout[3] = {1, 0, 254};
byte acZout[3] = {2, 0, 253};
byte piezoOut[3] = {3, 0, 254};

int highestValue = 0;
unsigned long timeSinceLastChange;
int lastTriggerValue;

void setup() {
  Serial.begin(9600);//enable serial monitor

  Wire.begin();
  Wire.beginTransmission(MPU1);
  Wire.write(0x6B);// PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  pinMode(13, OUTPUT);
  timeSinceLastChange = millis();
  lastTriggerValue = 0;

  pinMode(led1R, OUTPUT);
  pinMode(led1G, OUTPUT);
  pinMode(led1B, OUTPUT);

  pinMode(led2R, OUTPUT);
  pinMode(led2G, OUTPUT);
  pinMode(led2B, OUTPUT);

  digitalWrite(led1R, LOW);
  digitalWrite(led1G, LOW);
  digitalWrite(led1B, LOW);

  digitalWrite(led2R, LOW);
  digitalWrite(led2G, LOW);
  digitalWrite(led2B, LOW);
}

void loop() {

  GetMpuValue1(MPU1);
  Serial.print("  ");
  Serial.print("|||");

  int piezoValue = analogRead(14);//read analog value and put in to the variable

  if (piezoValue > 100 && lastTriggerValue == 0 && millis() - timeSinceLastChange > 50) {
    timeSinceLastChange = millis();
    lastTriggerValue = 1;
    piezoOut[1] = 1;
    //Serial.println(1);
    Serial.write(piezoOut, 3);
    digitalWrite(13, HIGH);
  } else if (lastTriggerValue == 1) {
    timeSinceLastChange = millis();
    lastTriggerValue = 0;
    piezoOut[1] = 0;
    //Serial.println(0);
    //Serial.println("---------");
    Serial.write(piezoOut, 3);
    digitalWrite(13, LOW);
  }
}

void GetMpuValue1(const int MPU) {

  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true); // request a total of 14 registers
  AcX1 = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY1 = Wire.read() << 8 |  Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ1 = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp1 = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX1 = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY1 = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ1 = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  acXout[1] = map((AcX1 / 16384.0 * 128), -128, 128, 0, 255);
  acYout[1] = map((AcY1 / 16384.0 * 128), -128, 128, 0, 256);
  acZout[1] = map((AcZ1 / 16384.0 * 128), -128, 128, 0, 256);
  Serial.write(acXout, 4);
  Serial.write(acYout, 4);
  Serial.write(acZout, 4);
  Serial.print("AcX = ");
  Serial.print(AcX1);
  Serial.print(" | AcY = ");
  Serial.print(AcY1);
  Serial.print(" | AcZ = ");
  Serial.print(AcZ1);
  Serial.print(" | GyX = ");
  Serial.print(GyX1);
  Serial.print(" | GyY = ");
  Serial.print(GyY1);
  Serial.print(" | GyZ = ");
  Serial.println(GyZ1);
}
