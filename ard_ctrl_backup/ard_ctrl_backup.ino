// #include <Arduino_JSON.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <comm_.h>
#include <TMC2209.h>
#include <SoftwareSerial.h>
#include "config.h"

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

  stepper_driver_2.setup (port2);
  stepper_driver_2.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver_2.enableCoolStep();
  stepper_driver_2.enable();
  
  stepper_driver_3.setup (port3);
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
  // Serial.print ("SPEED IS :");
  // Serial.println (SPEED);
  // Serial.println ("hello ");
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

  // Serial.println ("setting up inlets");
  ////////
  setup_outlets();
  ////////
  // Serial.println ("set up inlets");

 
}

void loop() {
  Serial.println("looping");
  // read serial commands 
  String cmd = readSerialInput();
  operate_inlets (cmd);

  ////
  
  // // Read PWM from JSON input
  // if (Serial.available()) {
  //   String input = Serial.readStringUntil('\n');
  //   StaticJsonDocument<256> doc;
  //   DeserializationError err = deserializeJson(doc, input);
  //   if (!err && doc.containsKey("pwm")) {
  //     JsonArray pwmArray = doc["pwm"];
  //     EFFLUENT_DUTY_1 = pwmArray[0] ;
  //     EFFLUENT_DUTY_2 = pwmArray[1] ;
  //     EFFLUENT_DUTY_3 = pwmArray[2] ;
  //     EFFLUENT_DUTY_4 = pwmArray[3] ;
  //     EFFLUENT_DUTY_5 = pwmArray[4] ;
  //     EFFLUENT_DUTY_6 = pwmArray[5] ;
  //     INLET_DUTY_1 = pwmArray[6] ;
  //     INLET_DUTY_2 = pwmArray[7] ;
  //   }

  //   if (!err & doc.containsKey ("estop")){

  //   }
  // }

  
  // int FLOAT_SWITCH_STATE_1 = digitalRead(FLOAT_SWITCH_PIN_1);
  // int FLOAT_SWITCH_STATE_2 = digitalRead(FLOAT_SWITCH_PIN_2);

  
  // // if (FLOAT_SWITCH_STATE_1 == HIGH){
  // //   EFFLUENT_DUTY_1 = min (100, EFFLUENT_DUTY_1 + KI_EFFLUENT_1);
  // //   EFFLUENT_DUTY_2 = min (100, EFFLUENT_DUTY_2 + KI_EFFLUENT_1);
  // //   EFFLUENT_DUTY_3 = min (100, EFFLUENT_DUTY_3 + KI_EFFLUENT_1);

  // //   INLET_DUTY_1 = max (0, INLET_DUTY_1 - KI_INLET_1);
  // // }
  // // else if (FLOAT_SWITCH_STATE_1 == LOW){
  // //   EFFLUENT_DUTY_1 = max (0, EFFLUENT_DUTY_1 - KI_EFFLUENT_1);
  // //   EFFLUENT_DUTY_2 = max (0, EFFLUENT_DUTY_2 - KI_EFFLUENT_1);
  // //   EFFLUENT_DUTY_3 = max (0, EFFLUENT_DUTY_3 - KI_EFFLUENT_1);

  // //   INLET_DUTY_1 =  min (1, INLET_DUTY_1 + KI_INLET_1);
  // // }

  // // if (FLOAT_SWITCH_STATE_2 == HIGH){
  // //   EFFLUENT_DUTY_4 = min (100, EFFLUENT_DUTY_4 + KI_EFFLUENT_2);
  // //   EFFLUENT_DUTY_5 = min (100, EFFLUENT_DUTY_5 + KI_EFFLUENT_2);
  // //   EFFLUENT_DUTY_6 = min (100, EFFLUENT_DUTY_6 + KI_EFFLUENT_2);

  // //   INLET_DUTY_2 = max (0, INLET_DUTY_2 - KI_INLET_2);
  // // }
  // // else if (FLOAT_SWITCH_STATE_2 == LOW){
  // //   EFFLUENT_DUTY_4 = max (0, EFFLUENT_DUTY_4 - KI_EFFLUENT_2);
  // //   EFFLUENT_DUTY_5 = max (0, EFFLUENT_DUTY_5 - KI_EFFLUENT_2);
  // //   EFFLUENT_DUTY_6 = max (0, EFFLUENT_DUTY_6 - KI_EFFLUENT_2);

  // //   INLET_DUTY_2 = min (100, INLET_DUTY_2 + KI_INLET_2);
  // // }

  

  // currentPWMValues[0] = EFFLUENT_DUTY_1 ;
  // currentPWMValues[1] = EFFLUENT_DUTY_2  ;
  // currentPWMValues[2] = EFFLUENT_DUTY_3  ;
  // currentPWMValues[3] = EFFLUENT_DUTY_4  ; 
  // currentPWMValues[4] = EFFLUENT_DUTY_5  ;
  // currentPWMValues[5] = EFFLUENT_DUTY_6  ;
  // currentPWMValues[6] = INLET_DUTY_1  ;
  // currentPWMValues[7] = INLET_DUTY_2  ;

  // // for (int i = 0; i < 6; i++){
  // //   stepper_drivers[i].moveAtVelocity (map_pwm_to_speed(currentPWMValues[i]));
  // //   delay(1);
  // //   // delay (10);
  // // }



  // // Read sensors and send as JSON
  // StaticJsonDocument<256> doc;
  // JsonObject root = doc.to<JsonObject>();

  // // save log every SAVE_INTERVAL milliseconds
  // int interval = millis() - prev_time;
  // if (interval >= SAVE_INTERVAL){
  //   root["log"] = "on";
  //   prev_time = millis();
  // }
  // else{ root ["log"] = "off"; }



  // // create JSON object
  // JsonArray TMP_data = root.createNestedArray("TMP_data");
  // for (int i = 0; i < 6; i++) {
  //   TMP_data.add(analogRead(TMP_pins[i]));
  // }
  // PH_1 = readPHValue (PH1_ADDR);
 
  // PH_2 = readPHValue (PH2_ADDR);

  // ORP_1 = readORPValue(ORP1_ADDR);

  // ORP_2 = readORPValue (ORP2_ADDR);
  

  // JsonArray PH_data = root.createNestedArray ("PH_data");
  // PH_data.add (PH_1);
  // PH_data.add (PH_2);

  // JsonArray ORP_data = root.createNestedArray ("ORP_data");
  // ORP_data.add (ORP_1);
  // ORP_data.add (ORP_2);

  // JsonArray pwmOutput = root.createNestedArray("pwm_output");
  // for (int i = 0; i < 8; i++) {
  //   pwmOutput.add (currentPWMValues[i]);
  // }


  // serializeJson(root, Serial);
  // Serial.println();
  delay(10);
}

