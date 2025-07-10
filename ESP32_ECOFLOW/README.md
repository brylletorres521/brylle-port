# EcoFLOW - Smart Irrigation System

## Project Overview

EcoFLOW is an IoT-based smart irrigation system designed for sugarcane fields. It monitors various soil parameters and provides irrigation recommendations based on the collected data.

## Features

- Real-time monitoring of soil parameters:
  - Temperature
  - Humidity
  - Conductivity
  - Salinity
  - pH level
- Water flow rate measurement during irrigation
- Automated irrigation recommendations based on soil data analysis
- Web-based dashboard for data visualization
- Mobile app for remote monitoring and control

## Hardware Components

- ESP32 microcontroller
- Soil moisture sensors
- Temperature and humidity sensors
- Conductivity sensors
- pH sensors
- Water flow sensors
- Irrigation control valves
- Power supply unit

## Software Architecture

The system consists of:
1. ESP32 firmware for sensor data collection and control
2. Backend server for data processing and storage
3. Web interface for visualization and management
4. Mobile application for remote access

## Installation and Setup

### Prerequisites
- Arduino IDE or PlatformIO
- ESP32 board support
- Required libraries (listed in the code)

### Steps
1. Clone this repository
2. Open the project in Arduino IDE or PlatformIO
3. Install the required libraries
4. Configure the WiFi credentials and server settings
5. Upload the firmware to your ESP32
6. Set up the sensors according to the wiring diagram
7. Power on the system and connect to your WiFi network

## Usage

After setup, the system will:
1. Collect soil data at regular intervals
2. Send data to the cloud server
3. Process the data and generate irrigation recommendations
4. Control irrigation valves based on recommendations or manual override

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contact

For more information, contact Brylle Andrei T. Torres at tbrylle7@gmail.com 