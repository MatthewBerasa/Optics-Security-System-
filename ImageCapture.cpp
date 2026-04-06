#include "core_fixes.h"
#include "arduino.h"
#include <ArduCAM.h>
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <HTTPClient.h>
#include "memorysaver.h"
#include "ImageCapture.h"
#include "ObjectDetected.h"
#include "WiFiHandle.h"

// Camera and webserver configurations
#define CS_PIN 5
#define I2C_SDA 21
#define I2C_SCL 22
ArduCAM myCAM(OV2640, CS_PIN);

// Multipart boundary string (can be any unique string)
const char* boundary = "----ESP32Boundary";

#define MAX_PAYLOAD_SIZE  60000  // 40 KB buffer for image upload
// Global buffer for image upload
uint8_t uploadBuffer[MAX_PAYLOAD_SIZE];  // Pre-allocate a buffer large enough 
uint32_t imageLength;


void imageCaptureSetup() {
  // Initialize I2C and SPI
  Wire.begin(I2C_SDA, I2C_SCL);
  SPI.begin();
  pinMode(CS_PIN, OUTPUT);

  // Initialize the camera
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  uint8_t test = myCAM.read_reg(ARDUCHIP_TEST1);
  if (test != 0x55) {
    Serial.println("Camera SPI interface error!");
    while (1);
  }
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.clear_fifo_flag();
  myCAM.OV2640_set_JPEG_size(OV2640_1024x768);  // Set resolution to 1024x768
  Serial.println("Resolution set to 1024x768");
  myCAM.OV2640_set_Brightness(1);  // Increase brightness (range: -2 to 2)
  myCAM.OV2640_set_Contrast(1);    // Increase contrast (range: -2 to 2)
  Serial.println("Camera initialized.");
}

void captureImage() {
  // Capture the image
  Serial.println("Capturing image...");
  myCAM.clear_fifo_flag();
  myCAM.start_capture();
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)); // Wait for capture

  imageLength = myCAM.read_fifo_length();
  if (imageLength == 0 || imageLength > 204800) { // Limit to 200 KB
    Serial.println("Image size error.");
    return;
  }
  Serial.print("Image captured. Size: ");
  Serial.println(imageLength);

  // Allocate buffer and read image data
  uint8_t *imageBuffer = (uint8_t *)malloc(imageLength);
  if (!imageBuffer) {
    Serial.println("Failed to allocate memory.");
    return;
  }
  
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  for (uint32_t i = 0; i < imageLength; i++) {
    imageBuffer[i] = SPI.transfer(0x00);
  }
  myCAM.CS_HIGH();

  systemSettings myPtr = {false, false};

  objectDetectionAlert(&myPtr);

  if(myPtr.notificationsEnabled){
    //Upload Image to Server
    Serial.println("Notifications On.");
    uploadImage(imageBuffer, imageLength);
  }

  free(imageBuffer); // Free allocated memory
}

void uploadImage(uint8_t *imgData, uint32_t imgLen) {
    const String URL = apiURL + "/addActivityLog";

    HTTPClient http;
    WiFiClient client;
    http.begin(client, URL);

    // Set multipart content type
    String boundary = "----ESP32Boundary";  // Define a static boundary
    String contentType = "multipart/form-data; boundary=" + boundary;
    http.addHeader("Content-Type", contentType);

    // Build multipart form-data payload manually
    String payloadHeader = "--" + boundary + "\r\n"
                           "Content-Disposition: form-data; name=\"deviceID\"\r\n\r\n" 
                           + deviceID + "\r\n"
                           "--" + boundary + "\r\n"
                           "Content-Disposition: form-data; name=\"image\"; filename=\"image.jpg\"\r\n"
                           "Content-Type: image/jpeg\r\n\r\n";

    String payloadFooter = "\r\n--" + boundary + "--\r\n";

    // Calculate total request length
    int totalLen = payloadHeader.length() + imgLen + payloadFooter.length();

    if (totalLen > MAX_PAYLOAD_SIZE) {
        Serial.println("Payload exceeds buffer size.");
        http.end();
        return;
    }

    // Fill the pre-allocated buffer with the payload
    int currentPos = 0;

    // Copy the header to the buffer
    memcpy(uploadBuffer + currentPos, payloadHeader.c_str(), payloadHeader.length());
    currentPos += payloadHeader.length();

    // Copy the image data to the buffer
    memcpy(uploadBuffer + currentPos, imgData, imgLen);
    currentPos += imgLen;

    // Copy the footer to the buffer
    memcpy(uploadBuffer + currentPos, payloadFooter.c_str(), payloadFooter.length());

    // Send the complete request in one go
    int httpResponseCode = http.sendRequest("POST", uploadBuffer, totalLen);

    // Read response
    String response = http.getString();
    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);
    Serial.print("Response: ");
    Serial.println(response);

    http.end();
}

void checkMemory() {
    if (ESP.getFreeHeap() < 20480) { // Restart if free memory is below 20 KB
        Serial.println("Low memory, restarting...");
        ESP.restart();
    }
}