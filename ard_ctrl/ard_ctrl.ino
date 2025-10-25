// #include <Arduino_JSON.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <comm_.h>
#include <TMC2209.h>
#include <SoftwareSerial.h>
#include "config.h"

int MODE = 0; // 0: RUN, 1, STOP, 2, CALIBRATE

// helpers 
int32_t map_pwm_to_speed (double pwm){
  int32_t speed = static_cast<int32_t> (pwm * STEP_CT/100);
  return speed ;
}

void setup_outlets() {
  
  /////////////

  stepper_driver_1.setup (port1);
  stepper_driver_1.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver_1.enableCoolStep();
  stepper_driver_1.enable();
  stepper_driver_1.setReplyDelay(8);
  stepper_drivers[0] = stepper_driver_1;

  stepper_driver_2.setup (port2);
  stepper_driver_2.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver_2.enableCoolStep();
  stepper_driver_2.enable();
  stepper_drivers[1] = stepper_driver_2;
  
  stepper_driver_3.setup (port3);
  stepper_driver_3.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver_3.enableCoolStep();
  stepper_driver_3.enable();
  stepper_driver_3.setReplyDelay(8);
  stepper_drivers[2] = stepper_driver_3;
  
  stepper_driver_4.setup (port4);
  stepper_driver_4.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver_4.enableCoolStep();
  stepper_driver_4.enable();
  stepper_driver_4.setReplyDelay(8);
  stepper_drivers[3] = stepper_driver_4;

  stepper_driver_5.setup (port5);
  stepper_driver_5.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver_5.enableCoolStep();
  stepper_driver_5.enable();
  stepper_driver_5.setReplyDelay(8);
  stepper_drivers[4] = stepper_driver_5;

  stepper_driver_6.setup (port6);
  stepper_driver_6.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver_6.enableCoolStep();
  stepper_driver_6.enable();
  stepper_driver_6.setReplyDelay(8);
  stepper_drivers[5] = stepper_driver_6;


  delay (10);

  
}

String readSerialInput() {
  String input = "";            // variable to hold the data

  // wait for user to type something and press Enter
  while (Serial.available() == 0) {
    // do nothing, just wait
  }

  // read everything that's available
  input = Serial.readStringUntil('\n');   // read until newline
  input.trim();                           // remove trailing \r or spaces
  return input;
}

void operate_inlets(String cmd){
  if (cmd == "PUMP1_ON"){
    digitalWrite (8, HIGH);
    Serial.println ("Turning on pump 1");
  }

  else if (cmd == "PUMP1_OFF"){
    digitalWrite (8, LOW);
    Serial.println ("Turning off pump 1");

  }

  else if  (cmd == "PUMP2_ON"){
    digitalWrite (9, HIGH);
    Serial.println ("Turning on pump 2");
  }

  else if (cmd == "PUMP2_OFF"){
    digitalWrite (9, LOW);
    Serial.println ("Turning off pump 2");

  }

  else if (cmd == "START"){
    Serial.println ("Starting pumps");
    stepper_driver_1.moveAtVelocity(map_pwm_to_speed (SPEED));
      delay (10);
    stepper_driver_2.moveAtVelocity(map_pwm_to_speed (SPEED));
      delay (10);
    stepper_driver_3.moveAtVelocity(map_pwm_to_speed (SPEED));
      delay (10);
    stepper_driver_4.moveAtVelocity(map_pwm_to_speed (SPEED));
      delay (10);
    stepper_driver_5.moveAtVelocity(map_pwm_to_speed (SPEED));
      delay (10);
    stepper_driver_6.moveAtVelocity(map_pwm_to_speed (SPEED));
      delay (10);
  }

  else if (cmd == "STOP"){
    Serial.println ("Stopping pumps");
     stepper_driver_1.moveAtVelocity(map_pwm_to_speed (0));
      delay (10);
    stepper_driver_2.moveAtVelocity(map_pwm_to_speed (0));
      delay (10);
    stepper_driver_3.moveAtVelocity(map_pwm_to_speed (0));
      delay (10);
    stepper_driver_4.moveAtVelocity(map_pwm_to_speed (0));
      delay (10);
    stepper_driver_5.moveAtVelocity(map_pwm_to_speed (0));
      delay (10);
    stepper_driver_6.moveAtVelocity(map_pwm_to_speed (0));
      delay (10);
  }

  else {
    // Serial.println ("Unrecognized command");
  }


}




void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 8; i++) {
    pinMode(motorPins[i], OUTPUT);
  }

  for (int i = 0; i < 6; i++) {
    pinMode(i ,OUTPUT);
  }

  pinMode (FLOAT_SWITCH_PIN_1, INPUT_PULLUP);
  pinMode (FLOAT_SWITCH_PIN_2, INPUT_PULLUP);

  prev_time = millis();
  Wire.begin(); // Initialize I2C communication
  activateSensor ( PH1_ADDR, 0x01);
  activateSensor ( PH2_ADDR, 0x01);
  activateSensor ( ORP1_ADDR, 0x01);
  activateSensor ( ORP2_ADDR, 0x01);

  setup_outlets();

  delay(10);
 
}

void _estop(){
  stepper_driver_1.moveAtVelocity(map_pwm_to_speed (0));
    delay (10);
  stepper_driver_2.moveAtVelocity(map_pwm_to_speed (0));
    delay (10);
  stepper_driver_3.moveAtVelocity(map_pwm_to_speed (0));
    delay (10);
  stepper_driver_4.moveAtVelocity(map_pwm_to_speed (0));
    delay (10);
  stepper_driver_5.moveAtVelocity(map_pwm_to_speed (0));
    delay (10);
  stepper_driver_6.moveAtVelocity(map_pwm_to_speed (0));
    delay (10);
  digitalWrite (8, LOW);
  digitalWrite (9, LOW);
}

void run_loop(  StaticJsonDocument<256> doc){
  if (doc.containsKey("pwm")) {
      JsonArray pwmArray = doc["pwm"];
      EFFLUENT_DUTY_1 = pwmArray[0] ;
      EFFLUENT_DUTY_2 = pwmArray[1] ;
      EFFLUENT_DUTY_3 = pwmArray[2] ;
      EFFLUENT_DUTY_4 = pwmArray[3] ;
      EFFLUENT_DUTY_5 = pwmArray[4] ;
      EFFLUENT_DUTY_6 = pwmArray[5] ;
      INLET_DUTY_1 = pwmArray[6] ;
      INLET_DUTY_2 = pwmArray[7] ;
    }

    

  
  int FLOAT_SWITCH_STATE_1 = digitalRead(FLOAT_SWITCH_PIN_1);
  int FLOAT_SWITCH_STATE_2 = digitalRead(FLOAT_SWITCH_PIN_2);
  

  currentPWMValues[0] = EFFLUENT_DUTY_1 ;
  currentPWMValues[1] = EFFLUENT_DUTY_2  ;
  currentPWMValues[2] = EFFLUENT_DUTY_3  ;
  currentPWMValues[3] = EFFLUENT_DUTY_4  ; 
  currentPWMValues[4] = EFFLUENT_DUTY_5  ;
  currentPWMValues[5] = EFFLUENT_DUTY_6  ;
  currentPWMValues[6] = INLET_DUTY_1  ;
  currentPWMValues[7] = INLET_DUTY_2  ;

  if (FLOAT_SWITCH_STATE_1 == HIGH) {
    currentPWMValues[6] = 0;
  }

  if (FLOAT_SWITCH_STATE_2 == HIGH) {
    currentPWMValues[7] = 0;
  }

  // Serial.println ("Operating pumps with PWM values:");
  // for (int i = 0; i < 8; i++) {
  //   Serial.print (currentPWMValues[i]);
  //   Serial.print (" ");
  // }
  // Serial.println();

  for (int i = 0; i < 6; i++){
    // Serial.println ("Setting pump " + String (i+1) + " to speed " + String (map_pwm_to_speed(currentPWMValues[i])));
    stepper_drivers[i].moveAtVelocity (map_pwm_to_speed(currentPWMValues[i]));
    delay(10);
    // delay (10);
  }

  analogWrite (motorPins[6], currentPWMValues[6] * 2.55); // scale 0-100 to 0-255
  analogWrite (motorPins[7], currentPWMValues[7] * 2.55); // scale 0-100 to 0-255

  // Read sensors and send as JSON
  doc.clear();
  JsonObject root = doc.to<JsonObject>();

  // save log every SAVE_INTERVAL milliseconds
  int interval = millis() - prev_time;
  if (interval >= SAVE_INTERVAL){
    root["log"] = "on";
    prev_time = millis();
  }
  else{ root ["log"] = "off"; }



  // create JSON object
  JsonArray TMP_data = root.createNestedArray("TMP_data");
  for (int i = 0; i < 6; i++) {
    TMP_data.add(analogRead(TMP_pins[i]));
  }
  PH_1 = readPHValue (PH1_ADDR);
 
  PH_2 = readPHValue (PH2_ADDR);

  ORP_1 = readORPValue(ORP1_ADDR);

  ORP_2 = readORPValue (ORP2_ADDR);
  

  JsonArray PH_data = root.createNestedArray ("PH_data");
  PH_data.add (PH_1);
  PH_data.add (PH_2);

  JsonArray ORP_data = root.createNestedArray ("ORP_data");
  ORP_data.add (ORP_1);
  ORP_data.add (ORP_2);

  JsonArray pwmOutput = root.createNestedArray("pwm_output");
  for (int i = 0; i < 8; i++) {
    pwmOutput.add (currentPWMValues[i]);
  }


  serializeJson(root, Serial);
  Serial.println();
}

void calibration (StaticJsonDocument<256> doc){
  if (doc.containsKey ("sensor_select")){
    String sensor = doc["sensor_select"].as<String>();
    if (sensor == "PH1"){
      if (doc.containsKey ("cal_value")){
        Serial.println ("Calibrating PH1"); 
        float cal_value = doc["cal_value"];
        sendPHCalibrationValues(PH1_ADDR, cal_value); 

        if (cal_value == 7.0){
          requestCalibration (PH1_ADDR, 1);
        }
        else if (cal_value == 4.0){
          requestCalibration (PH1_ADDR, 2);
        }
        else if (cal_value == 10.0){
          requestCalibration (PH1_ADDR, 3);
        }

        delay (1000);
        int code = confirmCalibration(PH1_ADDR);
        Serial.print ("Calibration status PH1: ");
        Serial.println(code);
      }   
    }
    else if (sensor == "PH2"){

      if (doc.containsKey ("cal_value")){
        Serial.println ("Calibrating PH2");
        float cal_value = doc["cal_value"];
        sendPHCalibrationValues(PH2_ADDR, cal_value); 

        if (cal_value == 7.0){
          requestCalibration (PH2_ADDR, 1);
        }
        else if (cal_value == 4.0){
          requestCalibration (PH2_ADDR, 2);
        }
        else if (cal_value == 10.0){
          requestCalibration (PH2_ADDR, 3);
        }

        delay (1000);
        int code = confirmCalibration(PH2_ADDR);
        Serial.print ("Calibration status PH2: ");
        Serial.println(code);
      }   
    }
    else if (sensor == "ORP1"){
      if (doc.containsKey ("cal_value")){
        Serial.println ("Calibrating ORP1");
        float cal_value = doc["cal_value"];
        sendORPCalibrationValues(ORP1_ADDR, cal_value); 
        requestCalibration (ORP1_ADDR, 2);
        delay (1000);
        int code = confirmCalibration(ORP1_ADDR);
        Serial.print ("Calibration status ORP1: ");
        Serial.println(code);
      }   
    }
    else if (sensor == "ORP2"){
      if (doc.containsKey ("cal_value")){
        Serial.println ("Calibrating ORP2");
        float cal_value = doc["cal_value"];
        sendORPCalibrationValues(ORP2_ADDR, cal_value); 
        requestCalibration (ORP2_ADDR, 2);
        delay (1000);
        int code = confirmCalibration(ORP2_ADDR);
        Serial.print ("Calibration status ORP2: ");
        Serial.println(code);
      }   
    }
  }
  

  // Read sensors and send as JSON
  doc.clear();
  JsonObject root = doc.to<JsonObject>();

  // don't save log in calibration mode
  root ["log"] = "off"; 

  // create JSON object
  JsonArray TMP_data = root.createNestedArray("TMP_data");
  for (int i = 0; i < 6; i++) {
    TMP_data.add(analogRead(TMP_pins[i]));
  }
  PH_1 = readPHValue (PH1_ADDR);
 
  PH_2 = readPHValue (PH2_ADDR);

  ORP_1 = readORPValue(ORP1_ADDR);

  ORP_2 = readORPValue (ORP2_ADDR);
  

  JsonArray PH_data = root.createNestedArray ("PH_data");
  PH_data.add (PH_1);
  PH_data.add (PH_2);

  JsonArray ORP_data = root.createNestedArray ("ORP_data");
  ORP_data.add (ORP_1);
  ORP_data.add (ORP_2);

  JsonArray pwmOutput = root.createNestedArray("pwm_output");
  for (int i = 0; i < 8; i++) {
    pwmOutput.add (0);
  }


  serializeJson(root, Serial);
  Serial.println();
}

// void loop(){
//   Serial.println ("Waiting for command...");
//   String cmd = readSerialInput();
//   operate_inlets (cmd);
//   delay (10);
// }

void loop() {
  // // Read PWM from JSON input
  
  String input = Serial.readStringUntil('\n');
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, input);
  if (!err & doc.containsKey ("estop") & doc.containsKey ("Calibration")){
    bool estop = doc["estop"];
    bool Calibration = doc["Calibration"];
    // if estop, stop all pumps
    if (estop){
      MODE = 1;
      Serial.println ("Emergency stop activated");
      _estop();
    }

    // if not estop and in estop mode, go to run mode
    else if (!estop & MODE == 1){
      Serial.println ("Exiting emergency stop");
      MODE = 0;
    }
    
    // if not in estop and Calibration true, go to calibration mode
    if (!estop & Calibration){
      Serial.println ("Entering calibration mode");
      MODE = 2;
    }

    // if not in estop and in calibration mode, go to run mode
    else if (!Calibration & MODE == 2){
      Serial.println ("Exiting calibration mode");
      MODE = 0;
    }

  }

  

  switch (MODE){
    case 0:
      run_loop (doc);
      break;
    case 1:
      // estop mode
      _estop();
      break;
    case 2:
      calibration (doc);
      break;
    default:
      // default to estop mode
      _estop();
      break;
  }

  
delay(10);
  
}




