/*
   1xPiezoSensor_values for ChimeTime using Teensy 3.2
   Reads values from a piezo representing vibration or kinetic pulses and passes them on over the serial port for use in MAX MSP.
   Connections:
   - Piezo to GRN
   - Piezo to A0 (pin 14 on Teensy 3.2)
   - 47k resistor between A0 and GND (or between the Piezo poles, it's physically the same thing)
*/

int highestValue = 0;
byte piezoOut[3] = {0, 0, 255};

void setup() {
  Serial.begin(9600);//enable serial monitor
  pinMode(13, OUTPUT);
}

void loop() {
  int value = analogRead(14);//read analog value and put in to the variable
  int mappedValue = map(value, 0, 1023, 0, 127);
  piezoOut[1] = mappedValue;
  Serial.write(piezoOut, 3);

  if (value > highestValue) {
    highestValue = value;
    Serial.println(highestValue);//print serial monitor
  }

  if (value > 100) {
    digitalWrite(13, HIGH);
  } else {
    digitalWrite(13, LOW);
  }
}
