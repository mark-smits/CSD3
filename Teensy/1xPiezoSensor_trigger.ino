/*
   1xPiezoSensor_trigger for ChimeTime using Teensy 3.2
   Reads values from a piezo and sends a '1' over the serial port when there is a kinetic pulse, followed by a zero as soon as the kinetic pulse has passed. Functions as a trigger.
   Connections:
   - Piezo to GRN
   - Piezo to A0 (pin 14 on Teensy 3.2)
   - 47k resistor between A0 and GND (or between the Piezo poles, it's physically the same thing)
*/

int highestValue = 0;
byte piezoOut[3] = {0, 0, 255};
unsigned long timeSinceLastChange;
int lastTriggerValue;

void setup() {
  Serial.begin(9600);//enable serial monitor
  pinMode(13, OUTPUT);
  timeSinceLastChange = millis();
  lastTriggerValue = 0;
}

void loop() {
  int value = analogRead(14);//read analog value and put in to the variable

  if (value > 100 && lastTriggerValue == 0 && millis() - timeSinceLastChange > 50) {
    timeSinceLastChange = millis();
    lastTriggerValue = 1;
    piezoOut[1] = 1;
    Serial.println(1);
    Serial.write(piezoOut, 3);
    digitalWrite(13, HIGH);
  } else if (lastTriggerValue == 1) {
    timeSinceLastChange = millis();
    lastTriggerValue = 0;
    piezoOut[1] = 0;
    Serial.println(0);
    Serial.println("---------");
    Serial.write(piezoOut, 3);
    digitalWrite(13, LOW);
  }
}
