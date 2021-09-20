#include <Arduino.h>

// parameters for the shift register
int bitarray[8] = {1,0,0,0,0,0,0,0};
int srclk_pin = 33;
int rclk_pin = 34;
int ser_pin = 35;
int delay_time = 8;
int ledarray[8][4] = {{}, {}, {}, {}, {}, {}, {}, {}};
int ledarrayIterator = 0;

void writeShiftRegister() {
  // write the bitarray to the shift register
  for (int i = 7; i>=0; i--)//for (int i = 7; i>=0; i--) for (int i = 0; i<8; i++)
  {
    digitalWrite(srclk_pin,LOW);
    delayMicroseconds(1);
    digitalWrite(rclk_pin,LOW);
    delayMicroseconds(1);
    if (bitarray[i] == 1)
    {
      digitalWrite(ser_pin,HIGH);
    }
    else
    {
      digitalWrite(ser_pin,LOW);
    }
    delayMicroseconds(1);
    digitalWrite(srclk_pin,HIGH);
    delayMicroseconds(1);
    digitalWrite(rclk_pin,HIGH);
    delayMicroseconds(1);
  }
}

void setLedValue(int led_nr, int brightness) {
  if (brightness == 0) {
    int bufferarray[4] = {0, 0, 0, 0};
    for (int i = 0; i < 4; i++){
      ledarray[led_nr][i] = bufferarray[i];
    }
  } else if (brightness == 1) {
    int bufferarray[4] = {1, 0, 0, 0};
    for (int i = 0; i < 4; i++){
      ledarray[led_nr][i] = bufferarray[i];
    }
  } else if (brightness == 2) {
    int bufferarray[4] = {1, 0, 1, 0};
    for (int i = 0; i < 4; i++){
      ledarray[led_nr][i] = bufferarray[i];
    }
  } else if (brightness == 3) {
    int bufferarray[4] = {1, 1, 1, 0};
    for (int i = 0; i < 4; i++){
      ledarray[led_nr][i] = bufferarray[i];
    }
  } else if (brightness == 4) {
    int bufferarray[4] = {1, 1, 1, 1};
    for (int i = 0; i < 4; i++){
      ledarray[led_nr][i] = bufferarray[i];
    }
  }
}

void updatebitarray(){
  while (ledarrayIterator > 3){
    ledarrayIterator = ledarrayIterator - 4;
  }
  for (int i = 0; i < 8; i++){
    bitarray[i] = ledarray[i][ledarrayIterator];
  }
  ledarrayIterator++;
}

void setup() {
  for (int i = 0; i < 8; i++){
    for (int j = 0; j < 4; j++){
      ledarray[i][j] = 0;
    }
  }
  for (int i = 0; i < 4; i++){
    setLedValue(i, i);
    setLedValue(i + 4, i);
  }
  Serial.begin(9600);
  pinMode(srclk_pin,OUTPUT);
  pinMode(rclk_pin,OUTPUT);
  pinMode(ser_pin,OUTPUT);
  writeShiftRegister();
}

void loop() {
  writeShiftRegister();
  delay(delay_time);
  updatebitarray();
}
