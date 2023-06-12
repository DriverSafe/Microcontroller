#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char *ssid = "";
const char *password = "";

const char *server = "www.googleapis.com";
const int port = 443;
const char *endpoint = "/geolocation/v1/geolocate?key="; // Replace with your API key

DynamicJsonDocument googleGeoLocation()
{
  HTTPClient http;
  DynamicJsonDocument responseJSON(1024); // Return JSON

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
  if (client.connect(server, port))
  {
    Serial.println("Connected to server");

    // Send the POST request with JSON payload
    client.print(String("POST ") + endpoint + " HTTP/1.1\r\n");
    client.print("Host: " + String(server) + "\r\n");
    client.print("Content-Type: application/json\r\n");
    client.print("Content-Length: " + String(jsonBody.length()) + "\r\n");
    client.print("\r\n");
    client.print(jsonBody);

    // Check for a successful response
    while (client.connected())
    {
      String line = client.readStringUntil('\n');
      if (line == "\r")
      {
        Serial.println("Headers received");
        break;
      }
    }

    String responseBody = "";
    // Read the response body
    while (client.available())
    {
      String line = client.readStringUntil('\n');
      responseBody += line;
      // Serial.println(line);
    }

    if (responseBody.length() != 0)
    {
      // Remove first numbers and last numbers
      int startIndex = responseBody.indexOf('{');
      int endIndex = responseBody.lastIndexOf('}');

      String realJSONstring = responseBody.substring(startIndex, endIndex + 1);

      // Add to new real JSON variable
      DeserializationError error = deserializeJson(responseJSON, realJSONstring);

      if (error)
      {
        Serial.print("JSON parsing error: ");
        Serial.println(error.c_str());
        client.stop();
      }
      else
      {
        Serial.println("JSON parsing complete!");
      }
    }

    // Disconnect from the server
    client.stop();
    Serial.println("Disconnected from server");
  }
  else
  {
    Serial.println("Connection failed");
  }

  return responseJSON;
}

void postRequestLocation()
{
  WiFiClientSecure client;
  client.setInsecure();
  if (client.connect("api.driversafe.tharindu.dev", 443))
  {
    Serial.println("Connected to server");

    // Create the JSON payload
    String jsonBody = "{\"lat\":10.00001,\"lng\":11.00001, \"accuracy\":12.00002}";

    // Send the POST request
    HTTPClient http;
    http.begin(client, "api.driversafe.tharindu.dev", 443, "/locations/");
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(jsonBody);

    // Check the HTTP response code
    if (httpCode > 0)
    {
      Serial.print("HTTP response code: ");
      Serial.println(httpCode);

      // Read the response
      String response = http.getString();
      Serial.print("Response: ");
      Serial.println(response);
    }
    else
    {
      Serial.println("Error sending POST request");
    }

    http.end(); // Close the connection
    Serial.println("Disconnected from server");
  }
  else
  {
    Serial.println("Connection failed");
  }
}

void setup()
{
  Serial.begin(115200);
  delay(100);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Wait for IP address
  while (WiFi.localIP() == INADDR_NONE)
  {
    delay(1000);
    Serial.println("Waiting for IP...");
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  delay(5000);

  // DynamicJsonDocument response = googleGeoLocation();

  // double x = response["location"]["lat"];
  // Serial.println(x);

  postRequestLocation();
}