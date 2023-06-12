#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "";
const char* password = "";

const char* server = "www.googleapis.com";
const int port = 443;
const char* endpoint = "/geolocation/v1/geolocate?key=";  // Replace with your API key

void setup() {
  Serial.begin(115200);
  delay(100);

  // Connect to Wi-Fi
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
  delay(10000);

  HTTPClient http;

  // Prepare JSON payload
  const int capacity = JSON_OBJECT_SIZE(6) + JSON_ARRAY_SIZE(0) + JSON_ARRAY_SIZE(0) + 70;
  DynamicJsonDocument jsonDoc(capacity);

  jsonDoc["homeMobileCountryCode"] = 413;
  jsonDoc["homeMobileNetworkCode"] = 8;
  jsonDoc["radioType"] = "gsm";
  jsonDoc["carrier"] = "Hutch Sri Lanka";
  jsonDoc["considerIp"] = true;
  jsonDoc.createNestedArray("cellTowers");
  jsonDoc.createNestedArray("wifiAccessPoints");

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

    // Read the response body
    while (client.available()) {
      String response = client.readStringUntil('\n');
      Serial.println(response);
    }

    // Disconnect from the server
    client.stop();
    Serial.println("Disconnected from server");
  } else {
    Serial.println("Connection failed");
  }
}