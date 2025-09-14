#include <TMC2209.h>
#include <SoftwareSerial.h>

// This example will not work on Arduino boards without HardwareSerial ports,
// such as the Uno, Nano, and Mini.
//
// See this reference for more details:
// https://www.arduino.cc/reference/en/language/functions/communication/serial/
//
// To make this library work with those boards, refer to this library example:
// examples/UnidirectionalCommunication/SoftwareSerial



const uint8_t RX_PIN = 50;
const uint8_t TX_PIN = 49;


HardwareSerial & port1  = Serial1;
SoftwareSerial port2 (RX_PIN, TX_PIN);
HardwareSerial & port3  = Serial2;
SoftwareSerial port4(52, 51);
HardwareSerial & port5  = Serial3;
SoftwareSerial port6(53, 48);


const int32_t RUN_VELOCITY = 40000;
const int32_t STOP_VELOCITY = 0;
const int RUN_DURATION = 2000;
const int STOP_DURATION = 1000;
// current values may need to be reduced to prevent overheating depending on
// specific motor and power supply voltage
const uint8_t RUN_CURRENT_PERCENT = 100;


// Instantiate TMC2209
TMC2209 stepper_driver_1, stepper_driver_2, stepper_driver_3, stepper_driver_4, stepper_driver_5, stepper_driver_6;

bool invert_direction = false;

void setup()
{
  

  stepper_driver_1.setup (Serial1);
  stepper_driver_1.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver_1.enableCoolStep();
  stepper_driver_1.enable();
  stepper_driver_1.setReplyDelay(8);

  stepper_driver_2.setup (port2);
  stepper_driver_2.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver_2.enableCoolStep();
  stepper_driver_2.enable();
  
  stepper_driver_3.setup (Serial3);
  stepper_driver_3.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver_3.enableCoolStep();
  stepper_driver_3.enable();
  stepper_driver_3.setReplyDelay(8);
  
  stepper_driver_4.setup (port4);
  stepper_driver_4.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver_4.enableCoolStep();
  stepper_driver_4.enable();

  stepper_driver_5.setup (port5);
  stepper_driver_5.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver_5.enableCoolStep();
  stepper_driver_5.enable();
  stepper_driver_5.setReplyDelay(8);

  stepper_driver_6.setup (port6);
  stepper_driver_6.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver_6.enableCoolStep();
  stepper_driver_6.enable();
  stepper_driver_6.setReplyDelay(8);


  delay (10);
  
 
  stepper_driver_1.moveAtVelocity(RUN_VELOCITY);
    delay (10);
  stepper_driver_2.moveAtVelocity(RUN_VELOCITY);
    delay (10);
  stepper_driver_3.moveAtVelocity(2*RUN_VELOCITY);
    delay (10);
  stepper_driver_4.moveAtVelocity(2*RUN_VELOCITY);
    delay (10);
  stepper_driver_5.moveAtVelocity(3*RUN_VELOCITY);
    delay (10);
  stepper_driver_6.moveAtVelocity(4*RUN_VELOCITY);
    delay (10);
}

void loop()
{
  // stepper_driver.moveAtVelocity(STOP_VELOCITY);
  // delay(STOP_DURATION);
  // if (invert_direction)
  // {
  //   stepper_driver.enableInverseMotorDirection();
  // }
  // else
  // {
  //   stepper_driver.disableInverseMotorDirection();
  // }
  // invert_direction = not invert_direction;

  // stepper_driver.moveAtVelocity(RUN_VELOCITY);
  // stepper_driver_1.moveAtVelocity(RUN_VELOCITY);

  // delay(RUN_DURATION);
}
