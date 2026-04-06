#include <Arduino.h>
#include "arduinoFFT.h"
#include "ObjectDetected.h"
#include "ImageCapture.h"
#include "WiFiHandle.h"

// Pin Definitions
#define LED_PWM_PIN    4      // GPIO4 for LED modulation
#define PHOTO_ADC_PIN  34     // GPIO36 for photodiode signal
#define ALARM 16

//Modulation Definitions
#define PWM_FREQUENCY 10000    //Modulation Frequency Hz
#define DETECT_FACTOR 1.5     //Factor to determine Threshold Detection
#define ALARM_PWM 1000 

//Calibration Definitions 
#define CALIBRATION_FREQUENCY 1000 //Sampling Frequency for Calibration 
#define CALIBRATION_SAMPLES 20 //Number of samples for Calibration 

//FFT Definitions
#define FFT_SAMPLES 128
#define FFT_SAMPLING_FREQ 20000  // 10 kHz

double vReal[FFT_SAMPLES];  // Real part of samples
double vImag[FFT_SAMPLES];  // Imaginary part (initialized to 0)

//Global Variables
float baseAmbientLight = 0; //Base Ambient Light ADC Reading from LED OFF
float threshold = 0; //Threshold for Detection 

// Create FFT object (note the corrected class name)
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, FFT_SAMPLES, FFT_SAMPLING_FREQ);

void setup() {
  Serial.begin(115200);

  // Alarm Modulation Configuration
  ledcSetup(1, ALARM_PWM, 8);
  ledcAttachPin(ALARM, 1);
  ledcWrite(1, 0);

  // LED Modulation Configuration 
  ledcSetup(0, PWM_FREQUENCY, 8);      // Channel 0, 1 kHz, 8-bit resolution
  ledcAttachPin(LED_PWM_PIN, 0);       //Attach GPIO4 to Channel 0
  ledcWrite(0, 0);                     // Start with LED off

  // ADC Configuration
  analogSetPinAttenuation(PHOTO_ADC_PIN, ADC_11db); //0-3.3 V Reading
  analogReadResolution(12); //0-4095 Resolution


  //Connect to WiFi and Initialize ImageCapture
  connectWiFi();
  imageCaptureSetup();

  //Calibrate Base Ambient Light 
  baseAmbientLight = calibrateLEDOff();
  delay(1000);
  //threshold = calibrateLEDON() * DETECT_FACTOR;
  //Serial.print("Threshold: ");
  //Serial.println(threshold);
}

void loop() {
  unsigned int nonZeroCounter = 0;
  unsigned int dipcounter = 0;
  do{
    sampleSignal();  // Collect 256 samples
    FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);  // Apply Hamming window
    FFT.compute(FFT_FORWARD);                         // Perform FFT
    FFT.complexToMagnitude();                         // Convert to amplitudes

    int bin = (10000 * FFT_SAMPLES) / FFT_SAMPLING_FREQ;  // Bin for 10 kHz (~26)
    float magnitude = vReal[bin];                // Get 10 kHz amplitude
    //Serial.print("10 kHz Magnitude: ");
    //Serial.println(magnitude);
    if(magnitude > 0){
      nonZeroCounter++;
      dipcounter = 0;
    }else{
      dipcounter++;
      if(dipcounter >= 2){
        nonZeroCounter = 0;
      }
    }
  }while(nonZeroCounter <= 4);

  captureImage(); //Capture Image
  Serial.println("ObjectDetected");

  //Keep alarm active while checkAlarmSounding() returns true
  while(checkAlarmSounding()){
   delay(500);
  }

  ledcWrite(1, 0); //Turn off Alarm

  delay(5000); //Prevent Rapid Retrigger
  Serial.println("System Ready!");
}

void sampleSignal() {
  unsigned long start = micros();
  unsigned int sampleCount = 0;
  while(sampleCount < FFT_SAMPLES){
    if(micros() - start >= (1000000 / FFT_SAMPLING_FREQ)) {
      vReal[sampleCount] = analogRead(PHOTO_ADC_PIN) - baseAmbientLight;  // Read TIA output Subtracting the DC offset Value from Ambient Light
      vImag[sampleCount] = 0;                          // No imaginary 
      sampleCount++;  
    }
  }
}

float calibrateLEDOff(){
  int sampleCounter = 0; 
  unsigned long lastSample = 0;
  unsigned int signalSum = 0;
  while(sampleCounter < CALIBRATION_SAMPLES){
    if(millis() - lastSample >= (1000/CALIBRATION_FREQUENCY)){
      lastSample = millis();
      sampleCounter++;
      signalSum += analogRead(PHOTO_ADC_PIN); 
    }
  }
  
  //Average Ambient Baseline ADC Reading
  Serial.print("Baseline Ambient Light (No LED): ");
  Serial.println(signalSum / sampleCounter);
  ledcWrite(0, 128); 
  return signalSum / sampleCounter;
}

float calibrateLEDON(){
  ledcWrite(0, 128); 
  float sumMagnitude = 0;
  unsigned int sampleCounter = 0;
  unsigned long lastTimeSampled = 0;
  while(sampleCounter < 750){
    if(micros() - lastTimeSampled >= (1000000/FFT_SAMPLING_FREQ)){
      sampleCounter++;
      lastTimeSampled = micros();
      sampleSignal();  // Function to collect 256 samples
      FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
      FFT.compute(FFT_FORWARD);
      FFT.complexToMagnitude();
      int bin = (10000 * FFT_SAMPLES) / FFT_SAMPLING_FREQ;  // Bin for 1 kHz
      sumMagnitude += vReal[bin];
    }
  }
  
  float baselineMagnitude = sumMagnitude / 750;
  Serial.print("Baseline 1 kHz Magnitude (No Object): ");
  Serial.println(baselineMagnitude);
  return baselineMagnitude;
}
