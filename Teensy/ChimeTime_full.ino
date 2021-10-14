/*
   Led1 R to pin 10, G to pin 9, B to pin 6
   Led2 R to pin 5, G to pin 4, B to pin 3

*/

#include <RGBLed.h>
#include<Wire.h>

RGBLed led1(10, 9, 6, RGBLed::COMMON_CATHODE);
RGBLed led2(5, 4, 3, RGBLed::COMMON_CATHODE);

const int MPU1 = 0x68;
int16_t AcX1, AcY1, AcZ1, Tmp1, GyX1, GyY1, GyZ1;
byte acXout[3] = {0, 0, 255};
byte acYout[3] = {1, 0, 254};
byte acZout[3] = {2, 0, 253};
byte piezoOut[3] = {3, 0, 252};

//int highestValue = 0;
unsigned long timeSinceLastChange;
int lastTriggerValue;
const int piezoActivityWindow = 2000;
unsigned long piezoActivityTimer;
bool stateHold;
int holdToggle;
int piezoValue;

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

const int piezoNumReadings = 500;
int piezoReadings[piezoNumReadings];
int piezoReadIndex = 0;
int piezoTotal = 0;
int piezoAverage = 0;

unsigned long lastLed1Update;
unsigned long lastLed2Update;
int r1;
int r2;
int targetR1;
int targetR2;
int g1;
int g2;
int targetG1;
int targetG2;
int b1;
int b2;
int targetB1;
int targetB2;

int additionR1;
int additionR2;
int additionG1;
int additionG2;
int additionB1;
int additionB2;

int state;

const int ledStepTime = 20;

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
  piezoActivityTimer = millis();
  stateHold = false;
  holdToggle = 0;

  r1 = 0;
  r2 = 0;
  g1 = 0;
  g2 = 0;
  b1 = 0;
  b2 = 0;
  targetR1 = 0;
  targetR2 = 0;
  targetG1 = 0;
  targetG2 = 0;
  targetB1 = 0;
  targetB2 = 0;
  additionR1 = 0;
  additionR2 = 0;
  additionG1 = 0;
  additionG2 = 0;
  additionB1 = 0;
  additionB2 = 0;
  lastLed1Update = millis();
  lastLed2Update = millis();

  piezoValue = 0;
  state = 5;
}

void loop() {
  GetMpuValue1(MPU1);
  SmoothMPU();
  //Serial.print("  ");
  //Serial.print("|||");

  piezoValue = analogRead(14);//read analog value and put in to the variable
  SmoothPiezo();
  Serial.print("Value: ");
  Serial.print(piezoValue);
  Serial.print('\t');
  

  if (piezoValue > 100 && lastTriggerValue == 0 && millis() - timeSinceLastChange > 50) {
    timeSinceLastChange = millis();
    lastTriggerValue = 1;
    piezoOut[1] = 1;

    //led1.setColor(random(0, 256), random(0, 256), random(0, 256));
    //led2.setColor(random(0, 256), random(0, 256), random(0, 256));
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

  if (millis() - piezoActivityTimer > piezoActivityWindow) {
    piezoActivityTimer = millis();
    if (piezoAverage < 40) {
      //idle state
      state = 0;
      targetR1 = 0;
      additionR1 = (targetR1 - r1) / 255;
      targetG1 = 0;
      additionG1 = (targetG1 - g1) / 255;
      targetB1 = 255;
      additionB1 = (targetB1 - b1) / 255;
      //led1.fadeIn(40, 40, 255, 20, piezoActivityWindow);
      //led2.fadeIn(40, 40, 255, 20, piezoActivityWindow);
    } else if (piezoAverage > 200) {
      //heave state
      state = 3;
      targetR1 = 255;
      additionR1 = (targetR1 - r1) / 255;
      targetG1 = 0;
      additionG1 = (targetG1 - g1) / 255;
      targetB1 = 0;
      additionB1 = (targetB1 - b1) / 255;
      //led1.fadeIn(255, 0, 0, 20, piezoActivityWindow);
      //led2.fadeIn(255, 0, 0, 20, piezoActivityWindow);
    } else if (100 < piezoAverage <= 200) {
      //medium state
      state = 2;
      targetR1 = 255;
      additionR1 = (targetR1 - r1) / 255;
      targetG1 = 255;
      additionG1 = (targetG1 - g1) / 255;
      targetB1 = 255;
      additionB1 = (targetB1 - b1) / 255;
      //led1.fadeIn(40, 40, 255, 20, piezoActivityWindow);
      //led2.fadeIn(255, 40, 40, 20, piezoActivityWindow);
    } else if (40 < piezoAverage <= 100) {
      //light state
      state = 1;
      targetR1 = 0;
      additionR1 = (targetR1 - r1) / 255;
      targetG1 = 255;
      additionG1 = (targetG1 - g1) / 255;
      targetB1 = 0;
      additionB1 = (targetB1 - b1) / 255;
      //led1.fadeIn(40, 40, 255, 20, piezoActivityWindow);
      //led2.fadeIn(200, 40, 100, 20, piezoActivityWindow);
    }
  }
  UpdateLed1();
  //Serial.write(piezoOut, 3);
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
/*
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
  */
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
  //Serial.write(acXout, 3);
  //Serial.write(acYout, 3);
  //Serial.write(acZout, 3);
}

void SmoothPiezo() {
  piezoTotal = piezoTotal - piezoReadings[piezoReadIndex];
  piezoReadings[piezoReadIndex] = piezoValue;
  piezoTotal = piezoTotal + piezoReadings[piezoReadIndex];
  piezoReadIndex = piezoReadIndex + 1;
  if (piezoReadIndex >= piezoNumReadings) {
    piezoReadIndex = 0;
  }
  piezoAverage = piezoTotal / piezoNumReadings;
  Serial.print("Average: ");
  Serial.print(piezoAverage);
  Serial.print('\n');
}

void UpdateLed1() {
  if (r1 != targetR1 && millis() - lastLed1Update > ledStepTime) {
    //lastLed1Update = millis();
    r1 += additionR1;
  } else if (r1 >= 255) {
    r1 = 255;
  } else if (r1 <= 0) {
    r1 = 0;
  }

  if (g1 != targetG1 && millis() - lastLed1Update > ledStepTime) {
    //lastLed1Update = millis();
    g1 += additionG1;
  } else if (g1 >= 255) {
    g1 = 255;
  } else if (g1 < 0) {
    g1 = 0;
  }

  if (b1 != targetB1 && millis() - lastLed1Update > ledStepTime) {
    //lastLed1Update = millis();
    b1 += additionB1;
  } else if (b1 >= 255) {
    b1 = 255;
  } else if (b1 <= 0) {
    b1 = 0;
  }

  led1.setColor(r1, g1, b1);
}
