#Power-Efficient Infrared Security System (PEISS)
This project is an optics-based security system designed for efficient and reliable object detection. The system uses dual infrared LEDs that emit modulated light. When an object enters the detection range, the emitted light
reflects off the object and returns to the photodiode. 
Upon detection, the system can notify the user through a mobile application and optionally trigger an alarm. 

##Overview
This section provides a description of the integrated software and hardware that make up the security system.

###Software
The system is divided into three main software components, each maintained in a separate repository:
####1. Embedded Firmware https://github.com/MatthewBerasa/Optics-Security-System-.git
The embedded firmware is implemented in C/C++ using the Arduino IDE for the ESP32-WROOM. 
Main Loop Logic Flow:
1. Sample photodiode signal using the ADC
2. Collect sufficient samples and perform a Fast-Fourier Transform (FFT)
3. Analyze the amplitude at the LED modulation frequency
4. Compare amplitude against a predefined threshold
   - If below threshold -> continue sampling
   - If above threshold -> object detected
If Object is Detected:
  1. Send API request to check user settings (alarm/notifications)
  2. Capture image using camera and send to server (if notifications enabled)
  3. Trigger alarm (if enabled)
  4. Continuously poll server to check if the user has disabled the alarm
  5. Return to sampling loop 

![alt text](https://imgur.com/a/uFs5RzL)

####2. API Backend https://github.com/MatthewBerasa/Optic-Security-System-APIs.git
The backend consists of:
- MongoDB for storing user credentials and detection logs
- Node.js/Express.js APIs (developed in VSCode)
- DigitalOcean server for hosting APIs and storing captured images

Available APIs:
- User Register (with verification via SendGrid)
- User Login (with password hashing)
- Update/Retrieve User Settings (notifications, alarm status)
- Add/Retrieve Detection Logs 
- Connect User to System
- Refresh JWT Authentication Token

####3. Mobile Frontend https://github.com/MatthewBerasa/Optic-Security-System-Mobile-Frontend.git
The mobile frontend is built using Flutter and developed in VSCode, supporting both Android and iOS.
The purpose of the mobile app is to allow users to interact with the security system by configuring system settings, view detection logs, and monitor activity. 

User Interfaces:
1. Sign In/Register
![alt-text](https://imgur.com/C9VqHM5)

2. Home Screen
![alt-text](https://imgur.com/fgkZEBp)

3. Detection Logs
![alt-text](https://imgur.com/EATMVUz)


###Hardware
The hardware is divided into three main subsystems: 
####1. Power Supply Unit
Designed for reliability, the system primarily operates on AC power with battery backup.
 - Automatic switching using an SPDT relay
 - Ensures seamless transition to battery during power outages
#####Components
- 120V AC -> 5V DC Converter 
- Battery Management System
- 3.3V Buck Regulator 

####2. Optical Hardware
This is the core sensing mechanism of the system.
- Dual Infrared LEDs
- Photodiode 
- Transimpedance Amplifier
- Optical Filter

####3. Microcontroller and Peripherals
These components integrate hardware and software into a complete system.
- ESP32-WROOM
- OV2640 Camera
- Magnetic Buzzer 

![alt-text](https://imgur.com/a/QxzS3dv) 





    

