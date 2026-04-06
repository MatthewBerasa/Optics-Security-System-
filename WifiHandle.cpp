#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiManager.h> 
#include <ArduinoJson.h>
#include "esp_wpa2.h"
#include "WiFiHandle.h"
#include "arduino.h"


//Connect to WiFi
void connectWiFi(){
  // Start WiFiManager
  WiFiManager wifiManager; 
  
  // Enable debug output
  wifiManager.setDebugOutput(true);
    
  // Show all networks regardless of signal strength
  wifiManager.setMinimumSignalQuality(0);

  // Uncomment this if you want to force WiFi setup every boot
  //wifiManager.resetSettings();

  if (!wifiManager.autoConnect("PEISS-System Setup", "PEISS_Spring2025")) {
    Serial.println("Failed to connect and timed out");
    ESP.restart();
  }
  
  Serial.println("Connected to WiFi!");
  updateWiFiState(true);
}


void updateWiFiState(bool state){
  const String URL = apiURL + "/updateSystemWiFiConnection"; // API URL
  HTTPClient http; // Establish instance of HTTP
  http.begin(URL); // Begin HTTP connection to API

  // Set the Content-Type header for POST request
  http.addHeader("Content-Type", "application/json");

  // Create JSON Payload, converting the boolean to a string literal
  String jsonBody = "{\"deviceID\":\"" + deviceID + "\", \"wifiState\":" + (state ? "true" : "false") + "}";

  int httpResponseCode = http.POST(jsonBody);

  if(httpResponseCode == 200){
    Serial.println("WiFi Connection Updated.");
  } else {
    Serial.print(httpResponseCode + " ");
    Serial.println("Failed to Update WiFi Connection.");
  }

  http.end();
}

bool checkConnectionStatus() {
  const String URL = apiURL + "/checkSystemWiFiConnection?deviceID=" + deviceID;
  HTTPClient http;
  http.begin(URL);
  http.addHeader("Content-Type", "application/json");
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
  bool result = (*doc)["status"]["wifiConnection"];
  delete doc;
  return result;
}

