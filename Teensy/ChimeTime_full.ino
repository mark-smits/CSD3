/*
   ChimeTime 17-OCT-2021
   Full version of Teensy code

   A simple overview of the functionality of the different components can be found in README.md in the Teensy folder in the Git repo. 
   Hardware connections can also be found in schematic form in ChimeTime_technical_schematic in the main folder of the repo.

   Hardware connections:

     LED's:
      LED1 R to pin 10, G to pin 9, B to pin 6 (82ohm resistors in series for each R, G and B pins), cathode to GND
      LED2 R to pin 5, G to pin 4, B to pin 3 (82ohm resistors in series for each R, G and B pins), cathode to GND

     Piezo's:
      - Piezo's to GND
      - Piezo's to pin 14, 15, 16, 17, 20 respectively
      - 47k resistor between A0 and GND (or between the Piezo poles, it's physically the same thing)

     MPU6050:
      - 3.3V to VCC
      - GND to GND
      - SCL to pin 19
      - SDA to pin 18
      - 4.7k pickup resistor between 3.3V and SCL
      - 4.7k pickup resistor between 3.3V and SDA

   Software connections:
   Unpack the 3 values from the serial object in MAX. 1st and 3rd integers are address, 2nd integer is the actual value associated to the variable.

     Accelerometer: (values from 0 to 256 with 128 representing 0)
      - acX is received on 0, 255
      - acY is received on 1, 254
      - acZ is received on 2, 253
     
     Piezos: (values are either 1 or 0 and can be send either as a stream or as triggers (look for instructions to comment/uncomment in code))
      - piezo1 is received on 3, 252
      - piezo2 is received on 4, 251
      - piezo3 is received on 5, 250
      - piezo4 is received on 6, 249
      - piezo5 is received on 7, 248
      - piezoAverage is received on 8, 247 (this is a value between 0 and 100 representing piezo trigger frequency aka 'how much are the pipes being hit or hitting eachother'
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
byte piezo1Out[3] = {3, 0, 252};
byte piezo2Out[3] = {4, 0, 251};
byte piezo3Out[3] = {5, 0, 250};
byte piezo4Out[3] = {6, 0, 249};
byte piezo5Out[3] = {7, 0, 248};
byte piezoAverageOut[3] = {8, 0, 247};

unsigned long timeSinceLastChange1;
int lastTriggerValue1 = 0;
unsigned long timeSinceLastChange2;
int lastTriggerValue2 = 0;
unsigned long timeSinceLastChange3;
int lastTriggerValue3 = 0;
unsigned long timeSinceLastChange4;
int lastTriggerValue4 = 0;
unsigned long timeSinceLastChange5;
int lastTriggerValue5 = 0;

const int piezoRetriggerBuffer = 50;

const int piezoActivityWindow = 500;
unsigned long piezoActivityTimer;

int piezoValue1 = 0;
int piezoValue2 = 0;
int piezoValue3 = 0;
int piezoValue4 = 0;
int piezoValue5 = 0;

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

float r1 = 0;
float r2 = 0;
int targetR1 = 0;
int targetR2 = 0;
float g1 = 0;
float g2 = 0;
int targetG1 = 0;
int targetG2 = 0;
float b1 = 0;
float b2 = 0;
int targetB1 = 0;
int targetB2 = 0;

float additionR1 = 0;
float additionR2 = 0;
float additionG1 = 0;
float additionG2 = 0;
float additionB1 = 0;
float additionB2 = 0;

int state = 0;
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

  timeSinceLastChange1 = millis();
  timeSinceLastChange2 = millis();
  timeSinceLastChange3 = millis();
  timeSinceLastChange4 = millis();
  timeSinceLastChange5 = millis();
  piezoActivityTimer = millis();
  lastZero = millis();
}

void loop() {
  GetMpuValue1(MPU1);
  SmoothMPU();
  //PrintMPU();

  piezoValue1 = analogRead(14);
  piezoValue2 = analogRead(15);
  piezoValue3 = analogRead(16);
  piezoValue4 = analogRead(17);
  piezoValue5 = analogRead(20);

  ProcessPiezos();
  //PrintPiezos();

  StateMachine();
  UpdateLeds();
  /*
    In case of piezo data streaming in stead of sending single 1/0 triggers, use/uncomment these:
    Serial.write(piezo1Out, 3);
    Serial.write(piezo2Out, 3);
    Serial.write(piezo3Out, 3);
    Serial.write(piezo4Out, 3);
    Serial.write(piezo5Out, 3);
  */
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
}

void PrintMPU() {
  Serial.print("  ");
  Serial.print("|||");
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

void ProcessPiezos() {
  // In case of piezo data streaming in stead of sending single 1/0 triggers, uncomment all Serial.write in this fucntion
  if (millis() - lastZero > zeroAddTimer) {
    AveragePiezo(0);
    lastZero = millis();
  }

  if (piezoValue1 > 100 && lastTriggerValue1 == 0 && millis() - timeSinceLastChange1 > piezoRetriggerBuffer) {
    timeSinceLastChange1 = millis();
    lastTriggerValue1 = 1;
    piezo1Out[1] = 1;
    AveragePiezo(1);
    Serial.write(piezo1Out, 3);
    digitalWrite(13, HIGH);
  } else if (lastTriggerValue1 == 1 && millis() - timeSinceLastChange1 > piezoRetriggerBuffer) {
    timeSinceLastChange1 = millis();
    lastTriggerValue1 = 0;
    piezo1Out[1] = 0;
    Serial.write(piezo1Out, 3);
    digitalWrite(13, LOW);
  }
  if (piezoValue2 > 100 && lastTriggerValue2 == 0 && millis() - timeSinceLastChange2 > piezoRetriggerBuffer) {
    timeSinceLastChange2 = millis();
    lastTriggerValue2 = 1;
    piezo2Out[1] = 1;
    AveragePiezo(1);
    Serial.write(piezo2Out, 3);
    digitalWrite(13, HIGH);
  } else if (lastTriggerValue2 == 1 && millis() - timeSinceLastChange2 > piezoRetriggerBuffer) {
    timeSinceLastChange2 = millis();
    lastTriggerValue2 = 0;
    piezo2Out[1] = 0;
    Serial.write(piezo2Out, 3);
    digitalWrite(13, LOW);
  }

  if (piezoValue3 > 100 && lastTriggerValue3 == 0 && millis() - timeSinceLastChange3 > piezoRetriggerBuffer) {
    timeSinceLastChange3 = millis();
    lastTriggerValue3 = 1;
    piezo3Out[1] = 1;
    AveragePiezo(1);
    Serial.write(piezo3Out, 3);
    digitalWrite(13, HIGH);
  } else if (lastTriggerValue3 == 1 && millis() - timeSinceLastChange3 > piezoRetriggerBuffer) {
    timeSinceLastChange3 = millis();
    lastTriggerValue3 = 0;
    piezo3Out[1] = 0;
    Serial.write(piezo3Out, 3);
    digitalWrite(13, LOW);
  }

  if (piezoValue4 > 100 && lastTriggerValue4 == 0 && millis() - timeSinceLastChange4 > piezoRetriggerBuffer) {
    timeSinceLastChange4 = millis();
    lastTriggerValue4 = 1;
    piezo4Out[1] = 1;
    AveragePiezo(1);
    Serial.write(piezo4Out, 3);
    digitalWrite(13, HIGH);
  } else if (lastTriggerValue4 == 1 && millis() - timeSinceLastChange4 > piezoRetriggerBuffer) {
    timeSinceLastChange4 = millis();
    lastTriggerValue4 = 0;
    piezo4Out[1] = 0;
    Serial.write(piezo4Out, 3);
    digitalWrite(13, LOW);
  }

  if (piezoValue5 > 100 && lastTriggerValue5 == 0 && millis() - timeSinceLastChange5 > piezoRetriggerBuffer) {
    timeSinceLastChange5 = millis();
    lastTriggerValue5 = 1;
    piezo5Out[1] = 1;
    AveragePiezo(1);
    Serial.write(piezo5Out, 3);
    digitalWrite(13, HIGH);
  } else if (lastTriggerValue5 == 1 && millis() - timeSinceLastChange5 > piezoRetriggerBuffer) {
    timeSinceLastChange5 = millis();
    lastTriggerValue5 = 0;
    piezo5Out[1] = 0;
    Serial.write(piezo5Out, 3);
    digitalWrite(13, LOW);
  }
}

void AveragePiezo(int piezoData) {
  piezoTotal = piezoTotal - piezoReadings[piezoReadIndex];
  piezoReadings[piezoReadIndex] = piezoData;
  piezoTotal = piezoTotal + piezoReadings[piezoReadIndex];
  piezoReadIndex = piezoReadIndex + 1;
  if (piezoReadIndex >= piezoNumReadings) {
    piezoReadIndex = 0;
  }
  piezoAverage = piezoTotal / piezoNumReadings;
  piezoAverageOut[1] = piezoAverage * 100;
  Serial.write(piezoAverageOut, 3);
}

void PrintPiezos() {
  Serial.print("Value1: ");
  Serial.print(piezoValue1);
  Serial.print('\t');
  Serial.print("Value2: ");
  Serial.print(piezoValue2);
  Serial.print('\t');
  Serial.print("Value3: ");
  Serial.print(piezoValue3);
  Serial.print('\t');
  Serial.print("Value4: ");
  Serial.print(piezoValue4);
  Serial.print('\t');
  Serial.print("Value5: ");
  Serial.print(piezoValue5);
  Serial.print('\t');
  Serial.print("Average: ");
  Serial.print(piezoAverage);
  Serial.print('\n');
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
    }
  }
}

void UpdateLeds() {
  if (r1 != targetR1) {
    r1 += additionR1;
  } else if (r1 >= 255) {
    r1 = 255;
  } else if (r1 <= 0) {
    r1 = 0;
  }

  if (g1 != targetG1) {
    g1 += additionG1;
  } else if (g1 >= 255) {
    g1 = 255;
  } else if (g1 <= 0) {
    g1 = 0;
  }

  if (b1 != targetB1) {
    b1 += additionB1;
  } else if (b1 >= 255) {
    b1 = 255;
  } else if (b1 <= 0) {
    b1 = 0;
  }

  if (r2 != targetR2) {
    r2 += additionR2;
  } else if (r2 >= 255) {
    r2 = 255;
  } else if (r2 <= 0) {
    r2 = 0;
  }

  if (g2 != targetG2) {
    g2 += additionG2;
  } else if (g2 >= 255) {
    g2 = 255;
  } else if (g2 <= 0) {
    g2 = 0;
  }

  if (b2 != targetB2) {
    b2 += additionB2;
  } else if (b2 >= 255) {
    b2 = 255;
  } else if (b2 <= 0) {
    b2 = 0;
  }
  led1.setColor(r1, g1, b1);
  led2.setColor(r2, g2, b2);
}
