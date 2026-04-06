// ObjectDetected.h
#ifndef OBJECT_DETECTED_H
#define OBJECT_DETECTED_H

typedef struct systemSettings{
  bool alarmEnabled;
  bool notificationsEnabled;
}systemSettings;


// Example function prototypes:
void checkSettings(systemSettings *ptr);
void objectDetectionAlert(systemSettings *ptr);
bool checkAlarmSounding();
void soundAlarm();
void wifiConnection();

// Other declarations (structures, constants, etc.)
#endif