#include <ArduinoJson.h>
#include <Wire.h>
#include <comm_.h>

uint8_t PH1_ADDR = 0x65;
uint8_t PH2_ADDR = 0x67;
uint8_t ORP1_ADDR = 0x66;
uint8_t ORP2_ADDR = 0x68;

uint8_t new_reading = 0x00;


void setup(){
    Serial.begin(9600);
    Wire.begin(); // Initialize I2C communication
    activateSensor(PH1_ADDR, 0x00); // Hibernate PH sensor 1
    activateSensor(PH2_ADDR, 0x00); // Hibernate PH sensor 2
    activateSensor(ORP1_ADDR, 0x00); // Hibernate ORP sensor 1
    activateSensor(ORP2_ADDR, 0x00); // Hibernate ORP sensor 2

    // // //Do calibration after reading stablizes 
    // sendPHCalibrationValues(PH2_ADDR, 10.0f); 
    // requestCalibration (PH2_ADDR, 4);
    // delay (1000);
    // int code = confirmCalibration(PH2_ADDR);
    // Serial.print ("Calibration status: ");
    // Serial.println(code);

    activateSensor(ORP2_ADDR, 0x01); // 

    // //Do calibration after reading stablizes 
    // sendORPCalibrationValues(ORP2_ADDR, 400.0f); 
    // requestCalibration (ORP1_ADDR, 1);
    delay (1000);
    int code = confirmCalibration(ORP2_ADDR);
    Serial.print ("Calibration status: ");
    Serial.println(code);

    
}

void loop(){

    uint8_t new_reading = readRegister(ORP1_ADDR, 0x07);
    if (new_reading == 0x01) {
        Serial.println("New reading available");
        writeRegister(ORP2_ADDR, 0x07, 0x00); // Clear the new reading flag
    }
    
    float value = readORPValue(ORP2_ADDR);
    Serial.print("ORP Value ");
    Serial.print(": ");
    Serial.println (value);
    delay (1000);


    // activateSensor(ORP1_ADDR, 0x00); // Hibernate ORP sensor 2

}