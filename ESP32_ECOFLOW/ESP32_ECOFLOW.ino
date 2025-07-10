#include <HardwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <LittleFS.h> // Include the LittleFS library
#include <regex>
#include <WiFiClientSecure.h>  // Required for secure connection

// Define hardware serial (use UART2)
HardwareSerial mySerial(2);

// Wi-Fi credentials
const char* ssid = "EcoFLOW";
const char* password = "123456789!!";

// Web server and WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Variables
unsigned long previous_millis = 0;
int pump = 14;
bool state = false; // State of the pump
String receivedData = "";
float moistureLevel = 0; // Current moisture level

// Global variables
volatile unsigned int on_seconds; // Volatile for shared access
char globalFlowRate[16];


int motor_state = 0;

int bypass = 0;

int justBoot = 1;

int recommendationFlag = 0;
// Function to check if a string is a valid floating-point number
bool isValidFloat(const String& str) {
  std::regex floatRegex(R"(^-?\d+(\.\d+)?$)"); // Allows positive/negative floats
  return std::regex_match(str.c_str(), floatRegex);
}

// Function to validate datetime format (YYYY-MM-DD HH:MM:SS.sssssss)
bool isValidDatetime(String datetime) {
  std::regex datetimeRegex(R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d+$)");
  return std::regex_match(datetime.c_str(), datetimeRegex);
}

// Function to validate the entire data entry
bool isValidDataEntry(String dataEntry) {

  Serial.println(dataEntry);
  // Split data entry into parts by ","
  int commaCount = 0;
  String values[8]; // Array to store split values
  int start = 0;
  int index = 0;

  for (int i = 0; i < dataEntry.length(); i++) {
    if (dataEntry[i] == ',') {
      values[index++] = dataEntry.substring(start, i);
      start = i + 1;
      commaCount++;
    }
  }
  values[index] = dataEntry.substring(start); // Last value

  // Ensure there are exactly 6 commas (7 values total)
  if (commaCount != 6) {
    Serial.println("Error: Incorrect number of values.");
    return false;
  }

  // Validate datetime format
  if (!isValidDatetime(values[0])) {
    Serial.println("Error: Invalid datetime format -> " + values[0]);
    return false;
  }

  // Validate numerical values (indexes 1 to 6)
  for (int i = 1; i <= 5; i++) {
    values[i].trim();  // Remove extra spaces
    if (values[i].length() == 0) {
      Serial.println("Error: Missing value at position " + String(i));
      return false;
    }
    if (!isValidFloat(values[i])) {
      Serial.println("Error: Invalid number detected -> " + values[i]);
      return false;
    }
  }

  // Validate the last value (should be "ON" or "OFF")
  values[6].trim();
  if (values[6] != "ON" && values[6] != "OFF") {
    Serial.println("Error: Invalid state -> " + values[6]);
    return false;
  }
  // If all checks pass
  return true;
}

// Function to check if the internet is actually available
bool isInternetAvailable() {
  WiFiClient client;
  const char* testURL = "http://clients3.google.com/generate_204"; // A fast, small HTTP request

  HTTPClient http;
  http.begin(client, testURL);
  int httpCode = http.GET();
  http.end();

  return (httpCode > 0 && httpCode < 400); // Success if we get a response
}

String fetchDatetime() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ No WiFi, using fallback datetime.");
    return "1970-01-01 00:00:00"; // Return default timestamp
  }

  // 2️⃣ Check if the internet is actually available
  if (!isInternetAvailable()) {
    Serial.println("❌ WiFi connected but no internet. Using fallback datetime.");
    return "1970-01-01 00:00:00"; // Return default timestamp
  }

  WiFiClientSecure client;
  HTTPClient https;
  String datetime = "1970-01-01 00:00:00"; // Default fallback

  client.setInsecure(); // Disable SSL certificate verification
  const char* apiUrl = "https://timeapi.io/api/time/current/zone?timeZone=Asia%2Fmanila";

  Serial.println("🌐 Fetching datetime from API...");

  // 3️⃣ Attempt to fetch datetime from API
  if (https.begin(client, apiUrl)) {
    int httpResponseCode = https.GET();

    if (httpResponseCode == 200) {
      String payload = https.getString();
      int startIndex = payload.indexOf("\"dateTime\":\"") + 12;
      int endIndex = payload.indexOf("\",", startIndex);

      if (startIndex > 10 && endIndex > startIndex) {
        datetime = payload.substring(startIndex, endIndex);
        datetime.replace("T", " ");
        Serial.println("✅ Fetched datetime: " + datetime);
      }
    } else {
      Serial.println("❌ Failed to fetch datetime. HTTP code: " + String(httpResponseCode));
    }

    https.end();
  } else {
    Serial.println("❌ Unable to connect to timeapi.io");
  }

  return datetime;
}


void eraseReadingsFile() {
  if (LittleFS.exists("/readings.txt")) { // Check if the file exists
    if (LittleFS.remove("/readings.txt")) { // Remove the file
      Serial.println("readings.txt has been erased successfully.");
    } else {
      Serial.println("Failed to erase readings.txt.");
    }
  } else {
    Serial.println("readings.txt does not exist.");
  }
}

void eraseReadingsFileWater() {
  if (LittleFS.exists("/water.txt")) { // Check if the file exists
    if (LittleFS.remove("/water.txt")) { // Remove the file
      Serial.println("readings.txt has been erased successfully.");
    } else {
      Serial.println("Failed to erase readings.txt.");
    }
  } else {
    Serial.println("readings.txt does not exist.");
  }
}

// Function to handle WebSocket messages
void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type,
                      void* arg, uint8_t* data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("WebSocket client connected");
  } else if (type == WS_EVT_DATA) {
    // Parse the received WebSocket message
    String message = String((char*)data).substring(0, len);
    Serial.println("Received WebSocket message: " + message);

    if (message == "ON") {
      state = true; // Turn the pump on
      digitalWrite(pump, HIGH);
      if (bypass == 1 && moistureLevel >= 73) {
        bypass = 2;
        Serial.println("Bypass == 2");
      }
      else if (bypass == 1 && moistureLevel < 73) {
        bypass = 0;
        Serial.println("Bypass == 0");
      }
      logData();
      notifyClients();
      on_seconds = 0;
      previous_millis = millis();
    } else if (message == "OFF") {
      state = false; // Turn the pump off
      logData();
      notifyClients();
      bypass = 0;
      Serial.println("Bypass == 0");
      Serial.print("Motor time: ");
      Serial.println(on_seconds);
      logWaterData();
      digitalWrite(pump, LOW);

    }
  }
}

// Function to log data to LittleFS
void logData() {
  String datetime = fetchDatetime(); // Fetch the current date and time
  if (receivedData.length() == 0) {
    Serial.println("Error: Received data is empty. Not saving.");
    return;
  }

  String dataToLog = datetime + "," + receivedData + "," + (state ? "ON" : "OFF");

  // Validate data format before logging
  if (isValidDataEntry(dataToLog)) {
    File file = LittleFS.open("/readings.txt", FILE_APPEND);
    if (file) {
      file.println(dataToLog);
      file.close();
      Serial.println("Data logged to readings.txt: " + dataToLog);
    } else {
      Serial.println("Failed to open readings.txt for writing.");
    }
  } else {
    Serial.println("Data entry validation failed. Not saving.");
  }
}

// Log water data
void logWaterData() {
  String datetime = fetchDatetime();
  if (datetime != "0") {
    char dataToLog[128];
    snprintf(dataToLog, sizeof(dataToLog), "%s,%u,%s", datetime.c_str(), on_seconds, globalFlowRate);
    File file = LittleFS.open("/water.txt", FILE_APPEND);
    if (file) {
      file.println(dataToLog);
      file.close();
      //Serial.println("Data logged to water.txt: " + String(dataToLog));
    } else {
      Serial.println("Failed to open water.txt for writing");
    }
  }
}

void setWaterSampleData() {
  String waterData =
    "2024-12-03 08:15:12,30,0.01\n"
    "2024-12-04 09:45:22,45,0.02\n"
    "2024-12-05 10:30:15,50,0.02\n"
    "2024-12-06 12:20:35,35,0.01\n"
    "2024-12-06 14:50:40,40,0.03\n"
    "2024-12-06 16:10:50,25,0.02\n"
    "2024-12-07 08:15:12,30,0.01\n"
    "2024-12-07 09:45:22,45,0.02\n"
    "2024-12-08 10:30:15,50,0.02\n"
    "2024-12-08 12:20:35,35,0.01\n"
    "2024-12-09 14:50:40,40,0.03\n"
    "2024-12-09 16:10:50,25,0.02\n"
    "2024-12-10 09:20:00,60,0.01\n"
    "2024-12-10 11:30:10,30,0.02\n"
    "2024-12-11 10:07:16,41,0.00\n"
    "2024-12-11 10:07:46,5,0.01";
  File file = LittleFS.open("/water.txt", FILE_WRITE);
  if (file) {
    file.println(waterData);
    file.close();
    Serial.println("Data logged to water.txt: " + waterData);
  } else {
    Serial.println("Failed to open water.txt for writing");
  }
}

void saveRecommendation(String recommendation) {
  recommendationFlag = 1;
  File file = LittleFS.open("/recommendation.txt", FILE_WRITE);
  file.println(recommendation);
  file.close();
}

void setSampleData() {
  String dataToLog =
    "2024-12-5 09:00:11,3.40, 25.00, 7.20, 3.00, 0.00,ON\n"
    "2024-12-5 09:00:16,2.90, 25.10, 7.00, 2.00, 0.00,OFF\n"
    "2024-12-5 09:00:21,4.10, 25.20, 6.80, 4.00, 0.00,ON\n"
    "2024-12-5 09:00:26,3.60, 25.00, 7.10, 2.00, 0.00,OFF\n"
    "2024-12-6 09:00:11,3.40, 25.00, 7.20, 5.00, 0.00,ON\n"
    "2024-12-6 09:00:16,2.90, 25.10, 7.00, 6.00, 0.00,OFF\n"
    "2024-12-6 09:00:21,4.10, 25.20, 6.80, 2.00, 0.00,ON\n"
    "2024-12-6 09:00:26,3.60, 25.00, 7.10, 4.00, 0.00,OFF\n"
    "2024-12-7 10:20:33,3.20, 25.50, 7.40, 2.00, 0.00,ON\n"
    "2024-12-7 10:20:38,4.00, 25.60, 7.20, 6.00, 0.00,OFF\n"
    "2024-12-7 10:20:43,3.90, 25.70, 7.10, 5.00, 0.00,ON\n"
    "2024-12-7 10:20:48,3.80, 25.50, 6.90, 3.00, 0.00,OFF\n"
    "2024-12-8 09:00:11,3.40, 25.00, 7.20, 4.00, 0.00,ON\n"
    "2024-12-8 09:00:16,2.90, 25.10, 7.00, 5.00, 0.00,OFF\n"
    "2024-12-8 09:00:21,4.10, 25.20, 6.80, 2.00, 0.00,ON\n"
    "2024-12-8 09:00:26,3.60, 25.00, 7.10, 7.00, 0.00,OFF\n"
    "2024-12-9 08:05:45,3.10, 24.70, 7.00, 2.00, 0.00,ON\n"
    "2024-12-9 08:05:50,3.80, 24.80, 7.40, 7.00, 0.00,OFF\n"
    "2024-12-9 08:05:55,4.00, 24.90, 7.10, 2.00, 0.00,ON\n"
    "2024-12-9 08:06:00,2.70, 24.70, 6.90, 3.00, 0.00,OFF\n"
    "2024-12-10 07:15:10,2.90, 25.10, 7.50, 4.00, 0.00,ON\n"
    "2024-12-10 07:15:15,3.70, 25.20, 6.90, 6.00, 0.00,OFF\n"
    "2024-12-10 07:15:20,4.20, 25.30, 7.00, 7.00, 0.00,ON\n"
    "2024-12-10 07:15:25,3.50, 25.10, 7.30, 2.00, 0.00,OFF\n"
    "2024-12-11 06:27:49,3.90, 24.90, 7.20, 1.00, 0.00,ON\n"
    "2024-12-11 06:27:54,3.10, 24.90, 6.80, 2.00, 0.00,OFF\n"
    "2024-12-11 06:27:59,2.10, 24.90, 7.10, 3.00, 0.00,ON\n"
    "2024-12-11 06:28:04,4.50, 24.90, 6.70, 4.00, 0.00,OFF";

  File file = LittleFS.open("/readings.txt", FILE_WRITE);
  if (file) {
    file.println(dataToLog);
    file.close();
    Serial.println("Data logged to readings.txt: " + dataToLog);
  } else {
    Serial.println("Failed to open readings.txt for writing");
  }
}

// Function to notify all WebSocket clients of the motor state and data
void notifyClients() {
  String message = "[" + receivedData + "]," + String(state ? "ON" : "OFF");
  ws.textAll(message);
}

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, 16, 17); // UART2: RX=16, TX=17
  // Setup pin
  pinMode(pump, OUTPUT);
  digitalWrite(pump, LOW);

  int retryCount = 0; // Initialize retry counter

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    retryCount++; // Increment retry counter

    if (retryCount >= 10) { // Check if retries exceed 10
      Serial.println("Failed to connect to WiFi after 10 attempts. Restarting...");
      ESP.restart(); // Restart the ESP32
    }
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("An error occurred while mounting LittleFS");
    return;
  }
  Serial.println("LittleFS mounted successfully");
  Serial.println("IP Address: " + WiFi.localIP().toString());

  File file = LittleFS.open("/recommendation.txt", FILE_WRITE);
  file.println("");
  file.close();

  // Serve the index.html file
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  // Serve the index.html file
  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest * request) {
    eraseReadingsFile();
    request->send(200, "text/plain", "OK"); // Send "ok" response to the client
  });

  // Serve the index.html file
  server.on("/deleteWater", HTTP_GET, [](AsyncWebServerRequest * request) {
    eraseReadingsFileWater();
    request->send(200, "text/plain", "OK"); // Send "ok" response to the client
  });

  // Serve the index.html file
  server.on("/soil_history", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/soil_history.html", "text/html");
  });

  // Serve the index.html file
  server.on("/water_history", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/water_history.html", "text/html");
  });


  // Serve the ecoflow.png image
  server.on("/ecoflow.png", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/ecoflow.png", "image/png");
  });

  // Serve the ecoflow.png image
  server.on("/webT.png", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/webT.png", "image/png");
  });

  // Serve the readings file
  server.on("/readings.txt", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/readings.txt", "text/plain");
  });

  // Serve the readings file
  server.on("/water.txt", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/water.txt", "text/plain");
  });

  // Serve the readings file
  server.on("/test.txt", HTTP_GET, [](AsyncWebServerRequest * request) {
    setSampleData(); // Call the function to set sample data (if necessary)
    request->send(200, "text/plain", "OK"); // Send "ok" response to the client
  });

  // Serve the readings file
  server.on("/testWater.txt", HTTP_GET, [](AsyncWebServerRequest * request) {
    setWaterSampleData(); // Call the function to set sample data (if necessary)
    request->send(200, "text/plain", "OK"); // Send "ok" response to the client
  });

  server.on("/saveRecommendation", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasParam("data")) {  // Check if 'data' parameter is present
      String recommendation = request->getParam("data")->value();
      saveRecommendation(recommendation);
      request->send(200, "text/plain", "Recommendation received");
    } else {
      request->send(400, "text/plain", "Missing 'data' parameter");
    }
  });

  // Serve the readings file
  server.on("/getRecommendation", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/recommendation.txt", "text/plain");
  });

  // Serve the readings file
  server.on("/setRecommendation", HTTP_GET, [](AsyncWebServerRequest * request) {
    File file = LittleFS.open("/recommendation.txt", FILE_WRITE);
    file.println("");
    file.close();
    request->send(200, "text/plain", "OK"); // Send "ok" response to the client
  });

  // Initialize WebSocket
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  // Start the server
  server.begin();


}

void loop() {
  // Check if data is received from Arduino Nano
  if (mySerial.available()) {
    receivedData = mySerial.readString(); // Read incoming data
    receivedData.trim(); // Remove extra whitespace or newline characters
    //Serial.print("Received from Arduino Nano: ");
    //Serial.println(receivedData);

    // Check if data is in the expected format
    if (receivedData.startsWith("[") && receivedData.endsWith("]")) {
      Serial.println("Data format is correct.");

      // Remove brackets
      receivedData = receivedData.substring(1, receivedData.length() - 1);

      // Extract moisture level (first value)
      String moistureString = receivedData.substring(0, receivedData.indexOf(","));
      moistureLevel = moistureString.toFloat();

      int lastCommaIndex = receivedData.lastIndexOf(",");
      String flowRateString = receivedData.substring(lastCommaIndex + 1);
      flowRateString.trim();
      float flowRate = flowRateString.toFloat() * 0.798;
      snprintf(globalFlowRate, sizeof(globalFlowRate), "%.3f", flowRate);

      // Notify WebSocket clients of the new data
      notifyClients();

      // Turn off the pump if moisture reaches or exceeds 70%
      if (moistureLevel >= 73 && state && bypass == 0) {
        Serial.println("STOP #1");
        state = false; // Turn the pump off
        logData();
        notifyClients(); // Update the webpage switch
        Serial.print("Motor time: ");
        Serial.println(on_seconds);
        logWaterData();
        digitalWrite(pump, LOW);
        bypass = 1;
        Serial.println("Bypass == 1");
      }
      else if (moistureLevel >= 85 && state && bypass == 2) {
        Serial.println("STOP #2");
        bypass = 1;
        state = false; // Turn the pump off
        logData();
        notifyClients(); // Update the webpage switch
        Serial.print("Motor time: ");
        Serial.println(on_seconds);
        logWaterData();
        bypass = 0;
        Serial.println("Bypass == 0");
        digitalWrite(pump, LOW);
      }

      if (justBoot == 1) {
        //logData();
        justBoot = 0;
      }
    } else {
      Serial.println("Data format is incorrect.");
    }
  }
  //Serial.println(bypass);
  if (state) {
    if (millis() - previous_millis >= 1000) {
      on_seconds++;
      previous_millis = millis();
    }
  }
  delay(50);
}