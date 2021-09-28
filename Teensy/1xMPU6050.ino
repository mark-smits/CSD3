/*
 * Reads 3-axis values from one MPU6050 accelerometer/gyro using I2C, which makes the code easily expandable for use of more than one MPU6050.
 * Connections:
 * - 3.3V to VCC
 * - GND to GND
 * - SCL to pin 19
 * - SDA to pin 18
 * - 4.7k pickup resistor between 3.3V and SCL
 * - 4.7k pickup resistor between 3.3V and SDA
 */

#include<Wire.h>
const int MPU1=0x68;
int16_t AcX1,AcY1,AcZ1,Tmp1,GyX1,GyY1,GyZ1;
byte acXout[4] = {0, 0, 0, 255};
byte acYout[4] = {1, 0, 0, 254};
byte acZout[4] = {2, 0, 0, 253};


 
//-------------------------------------------------\setup loop\------------------------------------------------------------ 
 void setup(){ 
      Wire.begin(); 
      Wire.beginTransmission(MPU1);
      Wire.write(0x6B);// PWR_MGMT_1 register 
      Wire.write(0); // set to zero (wakes up the MPU-6050)
      Wire.endTransmission(true);
      Serial.begin(9600); 
     }
     
//---------------------------------------------------\void loop\------------------------------------------------------------
 void loop(){
   
      //get values for first mpu having address of 0x68   
      GetMpuValue1(MPU1);
      Serial.print("  ");
      Serial.print("|||");
    }
 
//----------------------------------------------\user defined functions\-------------------------------------------------- 
      
 
 void GetMpuValue1(const int MPU){ 
   
      Wire.beginTransmission(MPU); 
      Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) 
      Wire.endTransmission(false);
      Wire.requestFrom(MPU, 14, true); // request a total of 14 registers 
      AcX1=Wire.read()<<8| Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L) 
      AcY1=Wire.read()<<8|  Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
      AcZ1=Wire.read()<<8| Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L) 
      Tmp1=Wire.read()<<8| Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L) 
      GyX1=Wire.read()<<8| Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L) 
      GyY1=Wire.read()<<8| Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L) 
      GyZ1=Wire.read()<<8| Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L) 
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
     
