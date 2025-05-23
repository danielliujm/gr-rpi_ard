#include <ArduinoJson.h>
#include <Wire.h>



const int motorPins[8] = {2, 3, 4, 5, 6, 7, 8, 9};
double PH_1 = 1.0;
double PH_2 = 2.0;
double ORP_1 = 3.0;
double ORP_2 = 4.0;

byte PH1_ADDR;
byte PH2_ADDR;
byte ORP1_ADDR;
byte ORP2_ADDR;


int prev_time; 
int SAVE_INTERVAL = 5000;

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 8; i++) {
    pinMode(motorPins[i], OUTPUT);
  }

  prev_time = millis();

}

void loop() {
  // Read PWM from JSON input
  
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, input);
    if (!err && doc.containsKey("pwm")) {
      JsonArray pwmArray = doc["pwm"];
      for (int i = 0; i < pwmArray.size(); i++) {
        analogWrite(motorPins[i], pwmArray[i]);
      }
    }
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

  JsonArray PH_data = root.createNestedArray ("PH_data");
  PH_data.add (PH_1);
  PH_data.add (PH_2);

  JsonArray ORP_data = root.createNestedArray ("ORP_data");
  ORP_data.add (ORP_1);
  ORP_data.add (ORP_2);

  serializeJson(root, Serial);
  Serial.println();
  delay(100);
}
