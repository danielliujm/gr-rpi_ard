#include <Wire.h>

byte DEFAULT_ADDR = 0x68;
byte UNLOCK_REG = 0x02;
byte ADDR_REG = 0x03;
byte NEW_ADDR = 0x66;

void setup(){
    Serial.begin(9600);
    Wire.begin();
    byte error;
    delay (3000);
    Serial.println("starting setup");
    // unlock address
    Wire.beginTransmission (DEFAULT_ADDR);
    Wire.write (UNLOCK_REG);
    Wire.write (0x55);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.println("unlocked register");
    }
    else if (error==4) 
    {
      Serial.println("failed unlocking register");
    } 

    Wire.beginTransmission (DEFAULT_ADDR);
    Wire.write (UNLOCK_REG);
    Wire.write (0xAA);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.println("unlocked register");
    }
    else if (error==4) 
    {
      Serial.println("failed unlocking register");
    }
    else{ Serial.println (error);}

    // change address
    Wire.beginTransmission (DEFAULT_ADDR);
    Wire.write (ADDR_REG);
    Wire.write (NEW_ADDR);
    Wire.endTransmission();
    
    Serial.println("finishing setup");



    

}

void loop(){
    byte error;

    // testing address change
    Wire.beginTransmission (NEW_ADDR);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.println("I2C device found at new address");
    }
    else if (error==4) 
    {
      Serial.println("No device fount at new address");
    } 
    else{
        Serial.println ("unknown error" + error);
    }
    delay (5000);
}