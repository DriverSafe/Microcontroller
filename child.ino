#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

// Network credentials
const char* ssid = "Dialog 4G";
const char* password = "";

// Google Geolocation API
const char* server = "www.googleapis.com";
const int port = 443;
const char* endpoint = "/geolocation/v1/geolocate?key=";

String lastRequest = "";

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Wait for IP address
  while (WiFi.localIP() == INADDR_NONE) {
    delay(1000);
    Serial.println("Waiting for IP...");
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.indexOf("request_") != -1) {
      Serial.println(command);
      if (lastRequest != command) {
        if (command != "request_send_location") {
          lastRequest = command;
        }
        
        if (command == "request_status_driving") {
          postStateRequest("Driving");
          Serial.println("Status sended");

        } else if (command == "request_status_accident") {
          postStateRequest("Accidented");
          Serial.println("Status sended");

        } else if (command == "request_status_break") {
          postStateRequest("Breaked");
          Serial.println("Status sended");

        } else if (command == "request_status_stopped") {
          postStateRequest("Stopped");
          Serial.println("Status sended");

        } else if (command == "request_send_location") {
          DynamicJsonDocument response = googleGeoLocation();
          double lat = response["location"]["lat"];
          double lng = response["location"]["lng"];
          double accuracy = response["accuracy"];

          if (lat != 0 && lng != 0) {
            postLocationRequest(lat, lng, accuracy);
          }
        }
      }
    }
  }
}

DynamicJsonDocument googleGeoLocation() {
  WiFi.mode(WIFI_STA);  // Enable station mode for WiFi

  HTTPClient http;
  DynamicJsonDocument responseJSON(1024);  // Return JSON

  // Prepare JSON payload
  const int capacity = JSON_OBJECT_SIZE(7) + JSON_ARRAY_SIZE(0) + JSON_ARRAY_SIZE(1) + 200;
  DynamicJsonDocument jsonDoc(capacity);

  jsonDoc["considerIp"] = false;

  // Create a nested array for wifiAccessPoints
  JsonArray wifiArray = jsonDoc.createNestedArray("wifiAccessPoints");

  // Scan for WiFi access points and add them to the payload
  int numOfNetworks = WiFi.scanNetworks();
  for (int i = 0; i < numOfNetworks; i++) {
    JsonObject wifiObj = wifiArray.createNestedObject();
    wifiObj["macAddress"] = WiFi.BSSIDstr(i);
    wifiObj["signalStrength"] = WiFi.RSSI(i);
  }

  String jsonBody;
  serializeJson(jsonDoc, jsonBody);

  // Start the HTTPS POST request
  WiFiClientSecure client;
  client.setInsecure();
  if (client.connect(server, port)) {
    Serial.println("Connected to server");

    // Send the POST request with JSON payload
    client.print(String("POST ") + endpoint + " HTTP/1.1\r\n");
    client.print("Host: " + String(server) + "\r\n");
    client.print("Content-Type: application/json\r\n");
    client.print("Content-Length: " + String(jsonBody.length()) + "\r\n");
    client.print("\r\n");
    client.print(jsonBody);

    // Check for a successful response
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("Headers received");
        break;
      }
    }

    String responseBody = "";
    // Read the response body
    while (client.available()) {
      String line = client.readStringUntil('\n');
      responseBody += line;
      // Serial.println(line);
    }

    Serial.println(responseBody);

    if (responseBody.length() != 0) {
      // Remove first numbers and last numbers
      int startIndex = responseBody.indexOf('{');
      int endIndex = responseBody.lastIndexOf('}');

      String realJSONstring = responseBody.substring(startIndex, endIndex + 1);

      // Add to new real JSON variable
      DeserializationError error = deserializeJson(responseJSON, realJSONstring);

      if (error) {
        Serial.print("JSON parsing error: ");
        Serial.println(error.c_str());
        client.stop();
      } else {
        Serial.println("JSON parsing complete!");
      }
    }

    // Disconnect from the server
    client.stop();
    Serial.println("Disconnected from server");
  } else {
    Serial.println("Connection failed");
  }

  return responseJSON;
}

void postStateRequest(String state) {
  WiFiClientSecure client;
  client.setInsecure();
  if (client.connect("api.driversafe.tharindu.dev", 443)) {
    Serial.println("Connected to server");

    // Create the JSON payload
    String jsonBody = "{\"speed\":0,\"state\":\"" + state + "\"}";

    // Send the POST request
    HTTPClient http;
    http.begin(client, "api.driversafe.tharindu.dev", 443, "/states/");
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(jsonBody);

    // Check the HTTP response code
    if (httpCode > 0) {
      Serial.print("HTTP response code: ");
      Serial.println(httpCode);

      // Read the response
      String response = http.getString();
      Serial.print("Response: ");
      Serial.println(response);
    } else {
      Serial.println("Error sending POST request");
    }

    http.end();  // Close the connection
    Serial.println("Disconnected from server");
  } else {
    Serial.println("Connection failed");
  }
}

void postLocationRequest(double lat, double lng, double accuracy) {
  WiFiClientSecure client;
  client.setInsecure();
  if (client.connect("api.driversafe.tharindu.dev", 443)) {
    Serial.println("Connected to server");

    // Create the JSON payload
    DynamicJsonDocument jsonDoc(256);
    jsonDoc["lat"] = lat;
    jsonDoc["lng"] = lng;
    jsonDoc["accuracy"] = accuracy;

    String jsonBody;
    serializeJson(jsonDoc, jsonBody);

    // Send the POST request
    HTTPClient http;
    http.begin(client, "api.driversafe.tharindu.dev", 443, "/locations/");
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(jsonBody);

    // Check the HTTP response code
    if (httpCode > 0) {
      Serial.print("HTTP response code: ");
      Serial.println(httpCode);

      // Read the response
      String response = http.getString();
      Serial.print("Response: ");
      Serial.println(response);
    } else {
      Serial.println("Error sending POST request");
    }

    http.end();  // Close the connection
    Serial.println("Disconnected from server");
  } else {
    Serial.println("Connection failed");
  }
}