### Project Description
This project is the final assignment for the "Internet of Things" lecture offered in the AI Bachelor program at Hochschule Fulda. The goal is to create a smart IoT device that can be controlled via an Android app, utilizing the MQTT protocol for communication.

---

### Project Goals
The project is divided into several phases, each with its own objectives and status:

- **Phase 1**: Adafruit's Neopixel library translation and implementation (for device needs). ✓
- **Phase 2**: RMT Protocol implementation. ✓
- **Phase 3**: MQTT Protocol implementation and Mosquitto Broker setup. ✓
- **Phase 4**: Android application development. ✓
- **Phase 5**: Testing and bug fixes. ✓

---

### Materials Needed
To replicate this project, you will need the following:

- **Hardware**:
  - ESP32 microcontroller.
  - WS2813 Mini (24 LED Ring).
  - Android phone (for testing the app).

- **Software**:
  - PlatformIO with the Espressif toolchain.
  - Mosquitto MQTT Broker (optional; a public broker can be used instead).

---

### Setup Instructions
#### 1. ESP32 Project
- An example of .json document structure is given in src folder.
---

##### main.c
main.c needs changes in function of your needs: BROKER_URI, AUTH_USR, AUTH_PWD, WIFI_SSID, WIFI_PASS.


Also, in main.c is managed wifi connection, by handling events. If a wifi connection is succesfully done (triggering IP_EVENT_STA_GOT_IP), then MQTT creation starts.

##### Libraries

There are several libraries, for organizing the code and implementing functions.

###### LED_DEFAULT_FUNCTIONS
Implements functions on advance for LEDs setup. For example, set flags, rainbow, happy face, ...

###### Paproka_LED_Lamp (There is an approach of modern RMT too in another file here, but is not usable/stable. Please use deprecated one)
Implements functions for controlling specified part of the lamp. This is the essential one, because manipulates everything related on what are we going to set on the lamp.
See .h for further information.

###### RMT_LED_Lamp
Implements deprecated RMT protocol for communicating with the lamp, and sending the information.

###### MQTT_Broker
Implements a client communication with broker. Base route starts with "topic/lamp/", and with usage of the wildcard #, we can add several things to this.
For example, we can:
topic/lamp/default_functions/ + message (check .h)
topic/lamp/json_function/ + file.json (or cat file.json as message). (Structure is also detailed in .h)

---

#### 2. Mosquitto broker
A mosquitto broker has been created for developing this project and for testing purposes.
For React Native's app, I also had to set up Websockets allowance.

Following this guide:
https://docs.vultr.com/install-mosquitto-mqtt-broker-on-ubuntu-20-04-server

And also setting up configuration for enabling WebSockets (got anonymous for time save. It has been tested with security) in mosquitto.conf file:

allow_anonymous true
#password_file /etc/mosquitto/password
listener 9001 0.0.0.0
protocol websockets

---

#### 3. Android App
Can be found here: https://github.com/FalseThinks/paproka-lamp-controller-mqtt.git

---

### Credits
Special thanks to:
- **Adafruit** for creating the Neopixel library.
- The **StackOverflow** and **GitHub** communities for their invaluable assistance during development.

---

### License
Using MIT License.

---

### Additional Notes
- The Android app repository will be linked here once available.
- If you encounter any issues, feel free to open an issue on GitHub.
