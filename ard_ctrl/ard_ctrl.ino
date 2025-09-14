#include <ArduinoJson.h>
#include <Wire.h>
#include <comm_.h>
#include <TMC2209.h>
#include <SoftwareSerial.h>



const int motorPins[8] = {2, 3, 4, 5, 6, 7, 8, 9};
double PH_1 = 1.0;
double PH_2 = 2.0;
double ORP_1 = 3.0;
double ORP_2 = 4.0;

uint8_t PH1_ADDR = 0x65;
uint8_t PH2_ADDR = 0x67;
uint8_t ORP1_ADDR = 0x66;
uint8_t ORP2_ADDR = 0x68;

int FLOAT_SWITCH_PIN_1 = 10;
int FLOAT_SWITCH_PIN_2 = 11;

double EFFLUENT_DUTY_1 = 0.0 ;
double EFFLUENT_DUTY_2 = 0.0 ;
double EFFLUENT_DUTY_3 = 0.0 ;
double EFFLUENT_DUTY_4 = 0.0 ;
double EFFLUENT_DUTY_5 = 0.0 ;
double EFFLUENT_DUTY_6 = 0.0 ;

double INLET_DUTY_1 = 0.0 ;
double INLET_DUTY_2 = 0.0 ;
double KI_EFFLUENT_1 = 0.0;
double KI_EFFLUENT_2 = 0.0;
double KI_INLET_1 = 2.0;
double KI_INLET_2 = 2.0;

double currentPWMValues[8] = {0,0,0,0,0,0,0,0};

int prev_time; 
int SAVE_INTERVAL = 10000;

const int RX_PIN_1 = 52;
const int TX_PIN_1 = 49;
const int RX_PIN_2 = 50;
const int TX_PIN_2 = 51;
const int RX_PIN_3 = 53;
const int TX_PIN_3 = 48;

TMC2209 stepper_drivers[6] = {};

const int RUN_CURRENT_PERCENT = 100; // Set the run current to 100% of the maximum



// helpers 
int map_pwm_to_speed (double pwm){
  return map (pwm, 0, 100, 0, 40000);
}

void setup_inlets() {
  
  HardwareSerial & port1  = Serial1;
  SoftwareSerial port2 (RX_PIN_1, TX_PIN_1);
  HardwareSerial & port3 = Serial2;
  SoftwareSerial port4(RX_PIN_2, TX_PIN_2);
  HardwareSerial & port5 = Serial3;
  SoftwareSerial port6(RX_PIN_3, TX_PIN_3);

  stepper_drivers[0].setup (port1);
  stepper_drivers[1].setup (port2);
  stepper_drivers[2].setup (port3);
  stepper_drivers[3].setup (port4);
  stepper_drivers[4].setup (port5);
  stepper_drivers[5].setup (port6);


  for (int i = 0; i< 6 ; i++){
  stepper_drivers[i].setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_drivers[i].enableCoolStep();
  stepper_drivers[i].enable();
  stepper_drivers[i].setReplyDelay(8);
  stepper_drivers[i].moveAtVelocity (20000);
  delay (100);

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

  setup_inlets();
}

void loop() {
  // Read PWM from JSON input
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, input);
    if (!err && doc.containsKey("pwm")) {
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
  }

  
  int FLOAT_SWITCH_STATE_1 = digitalRead(FLOAT_SWITCH_PIN_1);
  int FLOAT_SWITCH_STATE_2 = digitalRead(FLOAT_SWITCH_PIN_2);


  // if (FLOAT_SWITCH_STATE_1 == HIGH){
  //   EFFLUENT_DUTY_1 = min (100, EFFLUENT_DUTY_1 + KI_EFFLUENT_1);
  //   EFFLUENT_DUTY_2 = min (100, EFFLUENT_DUTY_2 + KI_EFFLUENT_1);
  //   EFFLUENT_DUTY_3 = min (100, EFFLUENT_DUTY_3 + KI_EFFLUENT_1);

  //   INLET_DUTY_1 = max (0, INLET_DUTY_1 - KI_INLET_1);
  // }
  // else if (FLOAT_SWITCH_STATE_1 == LOW){
  //   EFFLUENT_DUTY_1 = max (0, EFFLUENT_DUTY_1 - KI_EFFLUENT_1);
  //   EFFLUENT_DUTY_2 = max (0, EFFLUENT_DUTY_2 - KI_EFFLUENT_1);
  //   EFFLUENT_DUTY_3 = max (0, EFFLUENT_DUTY_3 - KI_EFFLUENT_1);

  //   INLET_DUTY_1 =  min (1, INLET_DUTY_1 + KI_INLET_1);
  // }

  // if (FLOAT_SWITCH_STATE_2 == HIGH){
  //   EFFLUENT_DUTY_4 = min (100, EFFLUENT_DUTY_4 + KI_EFFLUENT_2);
  //   EFFLUENT_DUTY_5 = min (100, EFFLUENT_DUTY_5 + KI_EFFLUENT_2);
  //   EFFLUENT_DUTY_6 = min (100, EFFLUENT_DUTY_6 + KI_EFFLUENT_2);

  //   INLET_DUTY_2 = max (0, INLET_DUTY_2 - KI_INLET_2);
  // }
  // else if (FLOAT_SWITCH_STATE_2 == LOW){
  //   EFFLUENT_DUTY_4 = max (0, EFFLUENT_DUTY_4 - KI_EFFLUENT_2);
  //   EFFLUENT_DUTY_5 = max (0, EFFLUENT_DUTY_5 - KI_EFFLUENT_2);
  //   EFFLUENT_DUTY_6 = max (0, EFFLUENT_DUTY_6 - KI_EFFLUENT_2);

  //   INLET_DUTY_2 = min (100, INLET_DUTY_2 + KI_INLET_2);
  // }



  currentPWMValues[0] = EFFLUENT_DUTY_1 ;
  currentPWMValues[1] = EFFLUENT_DUTY_2  ;
  currentPWMValues[2] = EFFLUENT_DUTY_3  ;
  currentPWMValues[3] = EFFLUENT_DUTY_4  ; 
  currentPWMValues[4] = EFFLUENT_DUTY_5  ;
  currentPWMValues[5] = EFFLUENT_DUTY_6  ;
  currentPWMValues[6] = INLET_DUTY_1  ;
  currentPWMValues[7] = INLET_DUTY_2  ;

  for (int i = 0; i < 6; i++){
    stepper_drivers[i].moveAtVelocity (map_pwm_to_speed (100.0));
    // delay (10);
  }



  // Read sensors and send as JSON
  StaticJsonDocument<256> doc;
  JsonObject root = doc.to<JsonObject>();

  int interval = millis() - prev_time;
  if (interval >= SAVE_INTERVAL){
    root["log"] = "on";
    prev_time = millis();
  }
  else{ root ["log"] = "off"; }


  JsonArray TMP_data = root.createNestedArray("TMP_data");
  for (int i = 0; i < 6; i++) {
    TMP_data.add(analogRead(i));
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
  delay(100);
}
