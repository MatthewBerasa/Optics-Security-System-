#include "arduino.h"
#include "ObjectDetected.h"
#include "ImageCapture.h"
#include <stdlib.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiManager.h> 
#include "esp_wpa2.h"
#include "WiFiHandle.h"

#define ALARM 16 


void objectDetectionAlert(systemSettings *ptr){
  //Pointer for Settings 
  checkSettings(ptr);  
    
  if(ptr->alarmEnabled){
    Serial.println("Alarm On");
    soundAlarm(); 
  }
}

void checkSettings(systemSettings *ptr) {
  const String URL = apiURL + "/provideSettings?deviceID=" + deviceID; //API URL
  
  HTTPClient http;
  http.begin(URL);
  
  int httpResponseCode = http.GET(); //API CALL
  if(httpResponseCode != 200){
    Serial.print("Error: ");
    Serial.println(httpResponseCode);
    ptr->alarmEnabled = false;
    ptr->notificationsEnabled = false;
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();
    
  // Parse JSON
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, payload);
    
  if (error) {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    ptr->alarmEnabled = ptr->notificationsEnabled = false;
    return;
  } 
  
  ptr->alarmEnabled = (bool)doc["alarmSetting"];
  ptr->notificationsEnabled = (bool)doc["notificationSetting"];
}

bool checkAlarmSounding() {
  const String URL = apiURL + "/getAlarmState?deviceID=" + deviceID;
  HTTPClient http;
  http.begin(URL);
  int httpResponseCode = http.GET();
  if (httpResponseCode != 200) {
    Serial.print("Error: "); Serial.println(httpResponseCode);
    http.end();
    return false;
  }
  String payload = http.getString();
  http.end();

  DynamicJsonDocument* doc = new DynamicJsonDocument(2048);
  if (!doc) {
    Serial.println("Failed to allocate JSON doc");
    return false;
  }
  DeserializationError error = deserializeJson(*doc, payload);
  if (error) {
    Serial.println("JSON Parse Failed: "); Serial.println(error.c_str());
    delete doc;
    return false;
  }
  bool result = (*doc)["alarmState"]["alarmSounding"];
  delete doc;
  return result;
}

void soundAlarm(){
  const String URL = apiURL + "/updateAlarmState";

  HTTPClient http;
  http.begin(URL);

  //Need Content-Type Header for POST Request
  http.addHeader("Content-Type", "application/json");

  //Create JSON payload
  String jsonBody = "{\"deviceID\":\"" + deviceID + "\", \"alarmState\":true}";

  int httpResponseCode = http.POST(jsonBody);

  if(httpResponseCode == 200){
    Serial.println("Alarm Sounding...");
    ledcWrite(1, 128);                     // Sound ALARM 
  }else{
    Serial.print("Error: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}