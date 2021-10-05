Teensy code for ChimeTime can be found here. To connect in MAX MSP load the COM port the Teensy is connected to. Output values range from 0 to 256 with 127 representing zero, the MPU6050 being flat and upright. Piezo triggers pass a single 1 followed by 0 when triggered. Hardware connections can be found in the Teensy code or in the pdf schematic.

Contents:

ChimeTime_full.ino
All functionality combined into one Arduino file. This is the (in progress) code that the Teensy will run for ChimeTime.

ChimeTime_full_50msSustainedTrigger
Is the same as the other full code, but sends piezo data as a stream instead of single 1 and 0 per trigger. Streams 1 for 50ms when triggered, then continuously sends 0's.

1xMPU6050.ino
Reads 3 axis from both the accelerometer and gyroscope, prints to the serial monitor and outputs accelerometer data over the USB serial bus. 

1xPiezoSensor_values.ino
Reads values from a piezo representing vibration or kinetic pulses and passes them on over the serial port for use in MAX MSP.

1xPiezoSensor_trigger.ino
Reads values from a piezo and sends a '1' over the serial port when there is a kinetic pulse, followed by a zero as soon as the kinetic pulse has passed. Functions as a trigger.
