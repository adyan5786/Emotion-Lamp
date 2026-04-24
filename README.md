# Emotion Lamp: Ambient Sound Driven Light Mood Visualizer

> **Abstract** : Emotion Lamp is a simple IoT project that changes light color and intensity based on surrounding sound. It uses a microphone to measure sound volume and frequency patterns and maps them to different light moods in real time. The project focuses on visualizing the overall sound environment of a room rather than detecting individual emotions, demonstrating basic signal processing and embedded system control.
> <br><br>
> Emotion Lamp is an embedded ambient-lighting system built to translate the sound character of a room into expressive visual output in real time. Instead of classifying emotions directly, it performs live audio analysis on the ESP32 using RMS loudness and dominant frequency, then maps those values to adaptive color zones, brightness levels, and animation behavior. The system combines on-device signal processing, customizable color profiles, and a browser-based control interface, allowing users to tune sensitivity, frequency bands, network settings, and visual modes without reflashing firmware. As a result, the project demonstrates a complete IoT workflow: sensing, processing, actuation, wireless control, and user-centric interaction design in a compact low-cost hardware setup.
> <br><br>
> Firmware can be flashed directly onto the ESP32 from the [Emotion Lamp installer](https://adyan5786.github.io/Emotion-Lamp/) (no Arduino IDE required) using ESP Web Tools and a supported browser.
> <br><br>
> Note: The software has only been configured for a 16-bit led ring light. 

## Screenshots

![Model](Resources/Screenshots%20and%20Demo%20Video/Model%20(Low%20Bass).png)

<p align="center">Model</p>

<table border="0">
 <tr>
    <td><b node-type="vertical-align: middle;">Home Page</b></td>
   <td><b node-type="vertical-align: middle;">Effects Page</b></td>
    <td><b node-type="vertical-align: middle;">Settings Page</b></td>
 </tr>
 <tr>
    <td><img src="Resources/Screenshots%20and%20Demo%20Video/Home%20Page.PNG" width="300" /></td>
    <td><img src="Resources/Screenshots%20and%20Demo%20Video/Effects%20Page%20-%201.PNG" width="300" /></td>
    <td><img src="Resources/Screenshots%20and%20Demo%20Video/Settings%20Page%20-%201.PNG" width="300" /></td>
 </tr>
</table>

[Click to view more...](Resources/Screenshots%20and%20Demo%20Video)

## Demo Video
[Download to view the demo video](Resources/Screenshots%20and%20Demo%20Video/Demo%20Video.mp4)

## Project Members
1. CHOUDHRY ABDUL REHMAN MOHD ASIM  [ Team Leader ]
2. PATHAN MUGAIRA ZAKEER
3. SADRIWALA MOHAMMED FIROZ
4. SHAIKH ADYAN ATAULLAH

## Project Guides
1. PROF. SHIBURAJ PAPPU  [ Primary Guide ]

## Deployment Steps


### Required Components (as per wiring diagram)
1. ESP32-WROOM-32 development board
2. INMP441 I2S microphone module
3. WS2812B 5V addressable LED strip/ring
4. SN74HCT125N logic level shifter IC
5. 68 ohm resistor (in LED data line)
6. 100 nF capacitor
7. 470 uF electrolytic capacitor
8. 5V DC fused power supply
9. Micro USB data cable (for flashing ESP32)
10. Jumper wires / interconnect wires

Assemble according to the diagram:

![Model Architecture Diagram](Resources/Screenshots%20and%20Demo%20Video/Model%20Architecture%20Diagram.jpg)

<p align="center">Model Architecture Diagram</p>

Please follow the below steps to run this project. 

### Method 1: One-Click Flash using GitHub Pages Installer (ESP Web Tools)
1. Open the [Emotion Lamp installer](https://adyan5786.github.io/Emotion-Lamp/) in a supported browser (lastest Chrome/Edge)
2. Connect ESP32 to laptop/PC using a Micro USB data cable.
3. Click Install and select the ESP32 serial port.
4. Allow erase/flash when prompted; wait for flashing to complete.
5. Go to Wifi and connect to the Emotion-Lamp network, entering the password as "akatsuki".
6. Open the browser and type in URL: 4.3.2.1 which opens up the Emotion-Lamp controller website. 
7. Open the website and go to Settings. In the Wi-Fi Configurator option, configure the Home Wi-Fi and hostname, then click Apply & Reboot.
8. After that connect to your Home Wi-Fi again and access the website through your hostname, adding a .local as the domain. (Example: emotionlamp.local)

### Method 2: Local Flash using Arduino IDE
1. Install the [Arduino IDE](https://www.arduino.cc/en/software)
2. Install the ESP32 board package (esp32 by Espressif Systems) from Board Manager
3. Install required libraries from Library Manager:
	- FastLED by Daniel Garcia
	- arduinoFFT by Enrique Condes 
4. Open emotion_lamp/emotion_lamp.ino and review settings in emotion_lamp/config.h (AP name/password, thresholds, frequency bands, pins if hardware differs).
5. Upload firmware on ESP32 and Verify.
6. After boot, go to Wifi and connect to the Emotion-Lamp network, entering the password as "akatsuki" (if unchanged).
7. Open the browser and type in URL: 4.3.2.1 (if unchanged) which opens up the Emotion-Lamp controller website.
8. Open the website and go to Settings. In the Wi-Fi Configurator option, configure the Home Wi-Fi and hostname, then click Apply & Reboot.
9. After that connect to your Home Wi-Fi again and access the website through your hostname, adding a .local as the domain. (Example: emotionlamp.local)

### Post-Deployment Verification (for both methods)
1. Check basic controls: power toggle, brightness, animation switching, and profile changes.
2. Open Settings -> Live Audio Monitor and confirm RMS/Hz values respond to room sound.
3. Open Settings -> System Specifications and verify firmware/runtime/network information.

## Subject Details
- Class : TE (COMP) Div A - 2025-2026
- Subject : Mini Project Lab: 2B (mP2B)
- Project Type : Mini Project

## Platform, Libraries and Frameworks used
1. [Arduino IDE](https://www.arduino.cc/en/software)
2. Board Manager: esp32 by Espressif Systems
3. Library Manager:
    - FastLED by Daniel Garcia
    - arduinoFFT by Enrique Condes 
4. C++ (Arduino Sketch Runtime)
5. WiFi.h (ESP32 Core)
6. ESPmDNS
7. Preferences (NVS Storage)
8. GitHub Actions + GitHub Pages (automated web installer deployment)
9. ESP Web Tools (browser-based flashing)

## References
- [WLED Project](https://kno.wled.ge/)
- [Arduino ESP32](https://docs.espressif.com/projects/arduino-esp32/)
- [FastLED](https://fastled.io/docs/)
- [Arduino FFT](https://github.com/kosme/arduinoFFT)
- [ESP Web Tools](https://esphome.github.io/esp-web-tools/)
- [Github Pages](https://docs.github.com/pages)
