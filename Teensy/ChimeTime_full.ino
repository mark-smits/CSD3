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
const int piezoActivityWindow = 500;
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

const int piezoNumReadings = 20;
int piezoReadings[piezoNumReadings];
int piezoReadIndex = 0;
float piezoTotal = 0.0f;
float piezoAverage = 0.0f;
const int zeroAddTimer = 500;
unsigned long lastZero;

unsigned long lastLed1Update;
unsigned long lastLed2Update;
float r1;
float r2;
int targetR1;
int targetR2;
float g1;
float g2;
int targetG1;
int targetG2;
float b1;
float b2;
int targetB1;
int targetB2;

float additionR1;
float additionR2;
float additionG1;
float additionG2;
float additionB1;
float additionB2;

int state;

const int ledStepTime = 100;
const int slowFactor = 10;

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
  lastZero = millis();
}

void loop() {
  GetMpuValue1(MPU1);
  SmoothMPU();

  //Serial.print("  ");
  //Serial.print("|||");

  piezoValue = analogRead(14);//read analog value and put in to the variable
  ProcessPiezo();
  Serial.print("Value: ");
  Serial.print(piezoValue);
  Serial.print('\t');
  Serial.print("Average: ");
  Serial.print(piezoAverage);
  Serial.print('\n');

  StateMachine();
  UpdateLeds();

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

void ProcessPiezo() {
  if (millis() - lastZero > zeroAddTimer) {
    SmoothPiezo(0);
    lastZero = millis();
  }

  if (piezoValue > 100 && lastTriggerValue == 0 && millis() - timeSinceLastChange > 50) {
    timeSinceLastChange = millis();
    lastTriggerValue = 1;
    piezoOut[1] = 1;
    SmoothPiezo(1);

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
}

void SmoothPiezo(int piezoData) {
  piezoTotal = piezoTotal - piezoReadings[piezoReadIndex];
  piezoReadings[piezoReadIndex] = piezoData;
  piezoTotal = piezoTotal + piezoReadings[piezoReadIndex];
  piezoReadIndex = piezoReadIndex + 1;
  if (piezoReadIndex >= piezoNumReadings) {
    piezoReadIndex = 0;
  }
  piezoAverage = piezoTotal / piezoNumReadings;
}

void StateMachine() {
  if (millis() - piezoActivityTimer > piezoActivityWindow) {
    piezoActivityTimer = millis();
    if (0 < piezoAverage && piezoAverage <= 0.20) {
      //light state
      state = 1;
      targetR1 = 0;
      additionR1 = (targetR1 - r1) / (255 * slowFactor);
      targetG1 = 0;
      additionG1 = (targetG1 - g1) / (255 * slowFactor);
      targetB1 = 255;
      additionB1 = (targetB1 - b1) / (255 * slowFactor);

      targetR2 = 0;
      additionR2 = (targetR2 - r2) / (255 * slowFactor);
      targetG2 = 255;
      additionG2 = (targetG2 - g2) / (255 * slowFactor);
      targetB2 = 0;
      additionB2 = (targetB2 - b2) / (255 * slowFactor);
      //led1.fadeIn(40, 40, 255, 20, piezoActivityWindow);
      //led2.fadeIn(200, 40, 100, 20, piezoActivityWindow);
    }
    if (piezoAverage <= 0) {
      //idle state
      state = 0;
      targetR1 = 0;
      additionR1 = (targetR1 - r1) / (255 * slowFactor);
      targetG1 = 0;
      additionG1 = (targetG1 - g1) / (255 * slowFactor);
      targetB1 = 255;
      additionB1 = (targetB1 - b1) / (255 * slowFactor);

      targetR2 = 0;
      additionR2 = (targetR2 - r2) / (255 * slowFactor);
      targetG2 = 0;
      additionG2 = (targetG2 - g2) / (255 * slowFactor);
      targetB2 = 255;
      additionB2 = (targetB2 - b2) / (255 * slowFactor);
      //led1.fadeIn(40, 40, 255, 20, piezoActivityWindow);
      //led2.fadeIn(40, 40, 255, 20, piezoActivityWindow);
    }
    if (piezoAverage > 0.70) {
      //heave state
      state = 3;
      targetR1 = 255;
      additionR1 = (targetR1 - r1) / (255 * slowFactor);
      targetG1 = 0;
      additionG1 = (targetG1 - g1) / (255 * slowFactor);
      targetB1 = 0;
      additionB1 = (targetB1 - b1) / (255 * slowFactor);

      targetR2 = 255;
      additionR2 = (targetR2 - r2) / (255 * slowFactor);
      targetG2 = 0;
      additionG2 = (targetG2 - g2) / (255 * slowFactor);
      targetB2 = 0;
      additionB2 = (targetB2 - b2) / (255 * slowFactor);
      //led1.fadeIn(255, 0, 0, 20, piezoActivityWindow);
      //led2.fadeIn(255, 0, 0, 20, piezoActivityWindow);
    }
    if (0.20 < piezoAverage && piezoAverage <= 0.70) {
      //medium state
      state = 2;
      targetR1 = 255;
      additionR1 = (targetR1 - r1) / (255 * slowFactor);
      targetG1 = 0;
      additionG1 = (targetG1 - g1) / (255 * slowFactor);
      targetB1 = 0;
      additionB1 = (targetB1 - b1) / (255 * slowFactor);

      targetR2 = 255;
      additionR2 = (targetR2 - r2) / (255 * slowFactor);
      targetG2 = 255;
      additionG2 = (targetG2 - g2) / (255 * slowFactor);
      targetB2 = 0;
      additionB2 = (targetB2 - b2) / (255 * slowFactor);
      //led1.fadeIn(40, 40, 255, 20, piezoActivityWindow);
      //led2.fadeIn(255, 40, 40, 20, piezoActivityWindow);
    }
  }
}

void UpdateLeds() {
  if (r1 != targetR1) {
    //lastLed1Update = millis();
    r1 += additionR1;
  } else if (r1 >= 255) {
    r1 = 255;
  } else if (r1 <= 0) {
    r1 = 0;
  }

  if (g1 != targetG1) {
    //lastLed1Update = millis();
    g1 += additionG1;
  } else if (g1 >= 255) {
    g1 = 255;
  } else if (g1 <= 0) {
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

  if (r2 != targetR2) {
    //lastLed1Update = millis();
    r2 += additionR2;
  } else if (r2 >= 255) {
    r2 = 255;
  } else if (r2 <= 0) {
    r2 = 0;
  }

  if (g2 != targetG2) {
    //lastLed1Update = millis();
    g2 += additionG2;
  } else if (g2 >= 255) {
    g2 = 255;
  } else if (g2 <= 0) {
    g2 = 0;
  }

  if (b2 != targetB2 && millis() - lastLed1Update > ledStepTime) {
    //lastLed1Update = millis();
    b2 += additionB2;
  } else if (b2 >= 255) {
    b2 = 255;
  } else if (b2 <= 0) {
    b2 = 0;
  }
  //lastLed1Update = millis();
  led1.setColor(r1, g1, b1);
  led2.setColor(r2, g2, b2);
}
