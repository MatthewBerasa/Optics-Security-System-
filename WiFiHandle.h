// WiFiHandle.h
#ifndef WIFI_HANDLE_H
#define WIFI_HANDLE_H

const String apiURL = "http://64.227.10.20:5000/api"; //API Base URL
const String deviceID = "674f1c233c69e373be41e8cf"; //Device ID for APIs

void connectWiFi();
void updateWiFiState(bool state);
bool checkConnectionStatus();

#endif