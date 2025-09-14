#ifndef SENSOR_CALIBRATION_H
#define SENSOR_CALIBRATION_H

#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>

// ----- Register Addresses -----
extern uint8_t INTERRUPT_CONTROL;
extern uint8_t LED_CONTROL;
extern uint8_t HIBERNATE_CONTROL;
extern uint8_t NEW_READING;
extern uint8_t CALIBRATION_MSB;
extern uint8_t CALIBRATION_VALUE_HI;
extern uint8_t CALIBRATION_VALUE_LO;
extern uint8_t CALIBRATION_LSB;
extern uint8_t CALIBRATION_CTRL;
extern uint8_t CALIBRATION_STATUS;

extern uint8_t PH_READING_MSB;
extern uint8_t PH_READING_HI;
extern uint8_t PH_READING_LO;
extern uint8_t PH_READING_LSB;

extern uint8_t ORP_READING_MSB;
extern uint8_t ORP_READING_HI;
extern uint8_t ORP_READING_LO;
extern uint8_t ORP_READING_LSB;

// ----- Function Declarations -----
int writeRegister(uint8_t add, uint8_t reg, uint8_t value);
uint8_t readRegister(uint8_t add, uint8_t reg);

int activateSensor(uint8_t add, uint8_t command);

void sendPHCalibrationValues(uint8_t add, float calibrationValue);
void sendORPCalibrationValues(uint8_t add, float calibrationValue);

int requestCalibration(uint8_t add, int commandValue);
int confirmCalibration(uint8_t add);

float readPHValue(uint8_t add);
float readORPValue(uint8_t add);

#endif // SENSOR_CALIBRATION_H