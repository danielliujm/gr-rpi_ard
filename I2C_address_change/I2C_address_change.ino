#include <Wire.h>

byte DEFAULT_ADDR = 0x65;
byte UNLOCK_REG = 0x02;
byte ADDR_REG = 0x03;
byte NEW_ADDR = 0x66;

void setup(){
    // unlock address
    Wire.beginTransmission (DEFAULT_ADDR);
    Wire.write (UNLOCK_REG);
    Wire.write (0x55);
    Wire.endTransmission();

    Wire.beginTransmission (DEFAULT_ADDR);
    Wire.write (UNLOCK_REG);
    Wire.write (0xAA);
    Wire.endTransmission();

    // change address
    Wire.beginTransmission (DEFAULT_ADDR);
    Wire.write (ADDR_REG);
    Wire.write (NEW_ADDR);
    Wire.endTransmission();

}

void loop(){

}