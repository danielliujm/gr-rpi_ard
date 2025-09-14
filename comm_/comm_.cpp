#include <ArduinoJson.h>
#include <Wire.h>
#include <stdint.h>
// #include <iostream>


uint8_t INTERRUPT_CONTROL = 0x04; // Address for interrupt control register
uint8_t LED_CONTROL = 0x05; // Address for LED control register
uint8_t HIBERNATE_CONTROL = 0x06; // Address for hibernate control register
uint8_t NEW_READING = 0x07; // Address for new reading available register

uint8_t CALIBRATION_MSB = 0x08; // Address for calibration MSB register
uint8_t CALIBRATION_VALUE_HI = 0x09; // Address for calibration value high byte
uint8_t CALIBRATION_VALUE_LO = 0x0A; // Address for calibration value low
uint8_t CALIBRATION_LSB = 0x0B; // Address for calibration LSB register

uint8_t CALIBRATION_CTRL = 0x0C; // Address for calibration control register
uint8_t CALIBRATION_STATUS = 0x0D; // Address for calibration status register

uint8_t PH_READING_MSB = 0x16; // Address for reading PH MSB register
uint8_t PH_READING_HI = 0x17; // Address for reading PH high byte
uint8_t PH_READING_LO = 0x18; // Address for reading PH low byte
uint8_t PH_READING_LSB = 0x19; // Address for reading PH LSB register

uint8_t ORP_READING_MSB = 0x0E; // Address for reading ORP MSB register
uint8_t ORP_READING_HI = 0x0F; // Address for reading ORP high byte
uint8_t ORP_READING_LO = 0x10; // Address for reading ORP low byte
uint8_t ORP_READING_LSB = 0x11; // Address for reading ORP LSB register

int writeRegister(uint8_t add, uint8_t reg, uint8_t value) {
    Wire.beginTransmission(add); // Replace with your device's I2C address
    Wire.write(reg);
    Wire.write(value);
    int err = Wire.endTransmission();
    if (err != 0) {
        Serial.println ("Error writing to register: ");
        // std::cerr << "Error writing to register: " << static_cast<int>(err) << std::endl;
    } 
    return err; // Return the error code
}

uint8_t readRegister (uint8_t add, uint8_t reg) {
    Wire.beginTransmission(add); 
    Wire.write(reg);
    int err = Wire.endTransmission();
    if (err != 0) {
        Serial.println ("Error writing to register: ");
        return err; // Return the error code
    }

    Wire.requestFrom(add, 1); // Request 1 byte from the device
    if (Wire.available() > 0) {
        uint8_t value = Wire.read(); // Read the byte
        return value; // Return the read value
    } else {
        Serial.println("No data available");
        return -1; // Indicate no data available
    }
    
    return 0; // Placeholder for actual implementation
}

int activateSensor (uint8_t add, uint8_t command){
    /*
    command 0x00 to hibernate
    command 0x01 to activate

    */

    int err = writeRegister(add, HIBERNATE_CONTROL, command);
    return err;
}

void sendPHCalibrationValues (uint8_t add, float calibrationValue){
    calibrationValue = calibrationValue * 1000.0f; // remove floating point numbers
    unsigned long calValue = static_cast<unsigned long>(calibrationValue);
    
    uint8_t calibrationValueMSB = (calValue >> 24) & 0xFF; // Get the MSB
    uint8_t calibrationValueHI = (calValue >> 16) & 0xFF; // Get the high byte
    uint8_t calibrationValueLO = (calValue >> 8) & 0xFF; // Get the low byte
    uint8_t calibrationValueLSB = (calValue >> 0) & 0xFF; // Get the LSB

    // std::cout << "Byte 0: " << static_cast<int>(calibrationValueMSB) << std::endl;
    // std::cout << "Byte 1: " << static_cast<int>(calibrationValueHI) << std::endl;
    // std::cout << "Byte 2: " << static_cast<int>(calibrationValueLO) << std::endl;
    // std::cout << "Byte 3: " << static_cast<int>(calibrationValueLSB) << std::endl;


    int MSB_out = writeRegister(add, CALIBRATION_MSB, calibrationValueMSB);
    int HI_out = writeRegister(add, CALIBRATION_VALUE_HI, calibrationValueHI);
    int LO_out = writeRegister(add, CALIBRATION_VALUE_LO, calibrationValueLO);
    int LSB_out =  writeRegister(add, CALIBRATION_LSB, calibrationValueLSB);

    if (MSB_out != 0 || HI_out != 0 || LO_out != 0 || LSB_out != 0) {
        Serial.println("Error writing calibration values");}

    return 0;

}


void sendORPCalibrationValues (uint8_t add, float calibrationValue){
    calibrationValue = calibrationValue * 10.0f; // remove floating point numbers
    unsigned long calValue = static_cast<unsigned long>(calibrationValue);
    
    uint8_t calibrationValueMSB = (calValue >> 24) & 0xFF; // Get the MSB
    uint8_t calibrationValueHI = (calValue >> 16) & 0xFF; // Get the high byte
    uint8_t calibrationValueLO = (calValue >> 8) & 0xFF; // Get the low byte
    uint8_t calibrationValueLSB = (calValue >> 0) & 0xFF; // Get the LSB

    // std::cout << "Byte 0: " << static_cast<int>(calibrationValueMSB) << std::endl;
    // std::cout << "Byte 1: " << static_cast<int>(calibrationValueHI) << std::endl;
    // std::cout << "Byte 2: " << static_cast<int>(calibrationValueLO) << std::endl;
    // std::cout << "Byte 3: " << static_cast<int>(calibrationValueLSB) << std::endl;


    int MSB_out = writeRegister(add, CALIBRATION_MSB, calibrationValueMSB);
    int HI_out = writeRegister(add, CALIBRATION_VALUE_HI, calibrationValueHI);
    int LO_out = writeRegister(add, CALIBRATION_VALUE_LO, calibrationValueLO);
    int LSB_out =  writeRegister(add, CALIBRATION_LSB, calibrationValueLSB);

    if (MSB_out != 0 || HI_out != 0 || LO_out != 0 || LSB_out != 0) {
        Serial.println("Error writing calibration values");}

    return 0;

}

int requestCalibration (uint8_t add, int commandValues){
    /* 
    commandValue = 1  clear all calibration values
    commandValue = 2  request low point calibration (ph = 4.0)  USE commandValue = 2 FOR ORP SINGLE POINT CALIBRATION
    commandValue = 3  request high point calibration (ph = 7.0)
    commandValue = 4  request mid point calibration (ph = 10.0)
    */

    int err = writeRegister(add, CALIBRATION_CTRL, commandValues);
    return err;
}


int confirmCalibration (uint8_t add){
    /*
    PH 
    HI              MID         LO       OUTPUT
    0               0           0          0
    0               0           1          1
    0               1           0          2
    0               1           1          3
    1               0           0          4
    1               0           1          5
    1               1           0          6
    1               1           1          7

    ORP 
    0       No calibration
    1       Calibration
    */
    uint8_t value = readRegister (add, CALIBRATION_STATUS);
    return static_cast<int>(value);

}

float readPHValue (uint8_t add){
    uint8_t msb = readRegister(add, PH_READING_MSB);
    uint8_t hi = readRegister(add, PH_READING_HI);
    uint8_t lo = readRegister(add, PH_READING_LO);
    uint8_t lsb = readRegister(add, PH_READING_LSB);

    unsigned long reading = (static_cast<unsigned long>(msb) << 24) |
                            (static_cast<unsigned long>(hi) << 16) |
                            (static_cast<unsigned long>(lo) << 8) |
                            static_cast<unsigned long>(lsb) << 0;

    float sensorValue = static_cast<float>(reading) / 1000.0f; // Convert back to float
    return sensorValue;
}

float readORPValue (uint8_t add){
    uint8_t msb = readRegister(add, ORP_READING_MSB);
    uint8_t hi = readRegister(add, ORP_READING_HI);
    uint8_t lo = readRegister(add, ORP_READING_LO);
    uint8_t lsb = readRegister(add, ORP_READING_LSB);

    signed long reading = (static_cast<signed long>(msb) << 24) |
                            (static_cast<signed long>(hi) << 16) |
                            (static_cast<signed long>(lo) << 8) |
                            static_cast<signed long>(lsb) << 0;

    float sensorValue = static_cast<float>(reading) / 10.0f; // Convert back to float
    return sensorValue;
}



