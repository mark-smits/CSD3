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
byte piezoOut[3] = {3, 0, 252};

int highestValue = 0;
unsigned long timeSinceLastChange;
int lastTriggerValue;

const int numReadings = 100;

int acXReadings[numReadings];
int acYReadings[numReadings];
int acZReadings[numReadings];
int readIndex = 0;
int acXTotal = 0;
int acXAverage = 0;
int acYTotal = 0;
int acYAverage = 0;
int acZTotal = 0;
int acZAverage = 0;

void setup() {
  Serial.begin(9600);//enable serial monitor
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    acXReadings[thisReading] = 0;
  }
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

  digitalWrite(led1R, HIGH);
  digitalWrite(led1G, HIGH);
  digitalWrite(led1B, HIGH);

  digitalWrite(led2R, HIGH);
  digitalWrite(led2G, HIGH);
  digitalWrite(led2B, HIGH);
}

void loop() {

  GetMpuValue1(MPU1);
  SmoothMPU();
  Serial.print("  ");
  Serial.print("|||");

  int piezoValue = analogRead(14);//read analog value and put in to the variable

  if (piezoValue > 100 && lastTriggerValue == 0 && millis() - timeSinceLastChange > 50) {
    timeSinceLastChange = millis();
    lastTriggerValue = 1;
    piezoOut[1] = 1;

    digitalWrite(led1R, random(0, 2));
    digitalWrite(led1G, random(0, 2));
    digitalWrite(led1B, random(0, 2));

    digitalWrite(led2R, random(0, 2));
    digitalWrite(led2G, random(0, 2));
    digitalWrite(led2B, random(0, 2));

    //Serial.println(1);
    
    digitalWrite(13, HIGH);
  } else if (lastTriggerValue == 1 && millis() - timeSinceLastChange > 50) {
    timeSinceLastChange = millis();
    lastTriggerValue = 0;
    piezoOut[1] = 0;
    //Serial.println(0);
    //Serial.println("---------");
    //Serial.write(piezoOut, 3);
    digitalWrite(13, LOW);
  }
  Serial.write(piezoOut, 3);
  
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

void SmoothMPU() {
  acXTotal = acXTotal - acXReadings[readIndex];
  acYTotal = acYTotal - acYReadings[readIndex];
  acZTotal = acZTotal - acZReadings[readIndex];

  acXReadings[readIndex] = AcX1;
  acYReadings[readIndex] = AcY1;
  acZReadings[readIndex] = AcZ1;

  acXTotal = acXTotal + acXReadings[readIndex];
  acYTotal = acYTotal + acYReadings[readIndex];
  acZTotal = acZTotal + acZReadings[readIndex];

  readIndex = readIndex + 1;

  if (readIndex >= numReadings) {
    readIndex = 0;
  }
  acXAverage = acXTotal / numReadings;
  acYAverage = acYTotal / numReadings;
  acZAverage = acZTotal / numReadings;

  acXout[1] = map((acXAverage / 16384.0 * 128), -128, 128, 0, 255);
  acYout[1] = map((acYAverage / 16384.0 * 128), -128, 128, 0, 255);
  acZout[1] = map((acZAverage / 16384.0 * 128), -128, 128, 0, 255);
  Serial.write(acXout, 3);
  Serial.write(acYout, 3);
  Serial.write(acZout, 3);
}
