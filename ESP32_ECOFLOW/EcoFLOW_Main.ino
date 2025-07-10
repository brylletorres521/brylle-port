/**
 * EcoFLOW - Smart Irrigation System for Sugarcane Fields
 * 
 * This firmware runs on an ESP32 microcontroller and handles:
 * - Soil parameter monitoring (temperature, humidity, conductivity, salinity, pH)
 * - Water flow rate measurement
 * - Data transmission to cloud server
 * - Irrigation control based on recommendations
 * 
 * Author: Brylle Andrei T. Torres
 * Contact: tbrylle7@gmail.com
 */

// Include necessary libraries
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DFRobot_ESP_PH.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>

// Display configuration
#define DISPLAY_SDA 21
#define DISPLAY_SCL 22

// Pin definitions
#define TEMP_SENSOR_PIN 4      // DS18B20 temperature sensor
#define HUMIDITY_SENSOR_PIN 36 // Soil moisture sensor (analog)
#define CONDUCTIVITY_PIN 39    // Conductivity sensor (analog)
#define PH_PIN 34             // pH sensor (analog)
#define FLOW_SENSOR_PIN 5      // Water flow sensor (digital)
#define VALVE_CONTROL_PIN 23   // Irrigation valve control relay

// Sensor objects
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature tempSensors(&oneWire);
DFRobot_ESP_PH ph;

// Global variables
float temperature = 0;
float humidity = 0;
float conductivity = 0;
float salinity = 0;
float phValue = 0;
float flowRate = 0;
volatile int flowPulseCount = 0;
unsigned long lastSensorReadTime = 0;
unsigned long lastDataSendTime = 0;
bool irrigationActive = false;

// Function declarations
void readSensors();
void calculateFlowRate();
bool analyzeDataForIrrigation();
void updateLocalDisplay();
void controlIrrigation(bool shouldIrrigate);
void IRAM_ATTR flowPulseCounter();

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("EcoFLOW System Initializing...");
  
  // Initialize EEPROM
  EEPROM.begin(32);
  
  // Initialize pH sensor
  ph.begin();
  
  // Initialize temperature sensor
  tempSensors.begin();
  
  // Setup pins
  pinMode(HUMIDITY_SENSOR_PIN, INPUT);
  pinMode(CONDUCTIVITY_PIN, INPUT);
  pinMode(PH_PIN, INPUT);
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  pinMode(VALVE_CONTROL_PIN, OUTPUT);
  
  // Turn off irrigation valve initially
  digitalWrite(VALVE_CONTROL_PIN, LOW);
  
  // Attach interrupt for flow sensor
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), flowPulseCounter, FALLING);
  
  // Initialize LCD display
  Wire.begin(DISPLAY_SDA, DISPLAY_SCL);
  // In a real implementation, you would initialize the LCD here
  // For example: lcd.begin(16, 2); lcd.init(); lcd.backlight();
  
  Serial.println("Display initialized");
  
  Serial.println("EcoFLOW System Ready");
}

void loop() {
  // Read sensors every 5 seconds
  if (millis() - lastSensorReadTime > 5000) {
    readSensors();
    lastSensorReadTime = millis();
    
    // Print sensor values to serial monitor
    Serial.println("Sensor Readings:");
    Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
    Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
    Serial.print("Conductivity: "); Serial.print(conductivity); Serial.println(" μS/cm");
    Serial.print("Salinity: "); Serial.print(salinity); Serial.println(" ppm");
    Serial.print("pH: "); Serial.println(phValue);
    Serial.print("Flow Rate: "); Serial.print(flowRate); Serial.println(" L/min");
  }
  
  // Process data and make irrigation decisions every minute
  if (millis() - lastDataSendTime > 60000) {
    // Analyze sensor data locally
    bool shouldIrrigate = analyzeDataForIrrigation();
    
    // Control irrigation based on local analysis
    controlIrrigation(shouldIrrigate);
    
    // Update display with current readings and recommendations
    updateLocalDisplay();
    
    lastDataSendTime = millis();
  }
  
  // Calculate flow rate if irrigation is active
  if (irrigationActive) {
    calculateFlowRate();
  }
}

void readSensors() {
  // Read temperature
  tempSensors.requestTemperatures();
  temperature = tempSensors.getTempCByIndex(0);
  
  // Read humidity (soil moisture)
  int humidityRaw = analogRead(HUMIDITY_SENSOR_PIN);
  humidity = map(humidityRaw, 4095, 1000, 0, 100); // Map raw value to percentage
  
  // Read conductivity
  int conductivityRaw = analogRead(CONDUCTIVITY_PIN);
  conductivity = conductivityRaw * 3.3 / 4095.0 * 1000; // Convert to μS/cm
  
  // Calculate salinity based on conductivity
  salinity = conductivity * 0.64; // Approximate conversion
  
  // Read pH
  float voltage = analogRead(PH_PIN) / 4096.0 * 3.3;
  phValue = ph.readPH(voltage, temperature);
}

void calculateFlowRate() {
  // Calculate flow rate based on pulse count
  flowRate = (flowPulseCount / 7.5); // Convert pulses to liters/minute
  flowPulseCount = 0; // Reset counter
}

bool analyzeDataForIrrigation() {
  // Local algorithm to determine if irrigation is needed based on sensor data
  
  // Check soil moisture - irrigate if too dry
  if (humidity < 30) {
    Serial.println("Soil moisture low - irrigation recommended");
    return true;
  }
  
  // Check temperature - irrigate if very hot
  if (temperature > 35) {
    Serial.println("High temperature detected - irrigation recommended");
    return true;
  }
  
  // Check soil conductivity - don't irrigate if too high (might indicate over-fertilization)
  if (conductivity > 2000) {
    Serial.println("High conductivity detected - irrigation not recommended");
    return false;
  }
  
  // Check pH - don't irrigate if pH is extreme
  if (phValue < 4.5 || phValue > 8.5) {
    Serial.println("Extreme pH detected - irrigation not recommended");
    return false;
  }
  
  // Default irrigation schedule based on time (example: irrigate every other day)
  unsigned long currentTime = millis();
  unsigned long dayInMillis = 86400000; // 24 hours in milliseconds
  if ((currentTime / dayInMillis) % 2 == 0) {
    Serial.println("Regular irrigation schedule - irrigation recommended");
    return true;
  }
  
  Serial.println("No irrigation needed at this time");
  return false;
}

void updateLocalDisplay() {
  // Update the local LCD display with current readings and recommendations
  Serial.println("Updating local display with current data");
  
  // In a real implementation, this would update an LCD or OLED display
  // For this example, we'll just print to Serial
  
  Serial.println("===== EcoFLOW STATUS =====");
  Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
  Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
  Serial.print("Conductivity: "); Serial.print(conductivity); Serial.println(" μS/cm");
  Serial.print("Salinity: "); Serial.print(salinity); Serial.println(" ppm");
  Serial.print("pH: "); Serial.println(phValue);
  Serial.print("Flow Rate: "); Serial.print(flowRate); Serial.println(" L/min");
  Serial.print("Irrigation: "); Serial.println(irrigationActive ? "ACTIVE" : "INACTIVE");
  Serial.println("==========================");
}

void controlIrrigation(bool shouldIrrigate) {
  if (shouldIrrigate && !irrigationActive) {
    // Turn on irrigation
    digitalWrite(VALVE_CONTROL_PIN, HIGH);
    irrigationActive = true;
    Serial.println("Irrigation started");
  } else if (!shouldIrrigate && irrigationActive) {
    // Turn off irrigation
    digitalWrite(VALVE_CONTROL_PIN, LOW);
    irrigationActive = false;
    Serial.println("Irrigation stopped");
  }
}

void IRAM_ATTR flowPulseCounter() {
  // Increment pulse counter
  flowPulseCount++;
} 