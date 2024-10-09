// FlipperHTTP.h for flipper-http.ino

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include <Arduino.h>
#include <ArduinoHttpClient.h>

#define B_PIN 4 // Blue
#define G_PIN 5 // Green
#define R_PIN 6 // Red

#define ON  LOW
#define OFF HIGH

class FlipperHTTP {
public:
    // Constructor
    FlipperHTTP() {
    }

    // Main methods for flipper-http.ino
    void setup();
    void loop();

    // HTTP Methods
    String get(String url);
    String get(String url, const char* headerKeys[], const char* headerValues[], int headerSize);
    String post(String url, String payload);
    String post(
        String url,
        String payload,
        const char* headerKeys[],
        const char* headerValues[],
        int headerSize);
    String put(String url, String payload);
    String
        put(String url,
            String payload,
            const char* headerKeys[],
            const char* headerValues[],
            int headerSize);
    String delete_request(String url, String payload);
    String delete_request(
        String url,
        String payload,
        const char* headerKeys[],
        const char* headerValues[],
        int headerSize);

    // Save and Load settings to and from SPIFFS
    bool saveWifiSettings(String data);
    bool loadWifiSettings();

    // Connect to Wifi using the loaded SSID and Password
    bool connectToWifi();

    // Check if the Dev Board is connected to Wifi
    bool isConnectedToWifi() {
        return WiFi.status() == WL_CONNECTED;
    }

    // Read serial data until newline character
    String readSerialLine();

    // Clear serial buffer to avoid any residual data
    void clearSerialBuffer() {
        while(Serial.available() > 0) {
            Serial.read();
        }
    }

    // Turn on and off the LED
    void ledAction(int pin = G_PIN, int timeout = 250) {
        digitalWrite(pin, ON);
        delay(timeout);
        digitalWrite(pin, OFF);
        delay(timeout);
    }

    // Display LED sequence when Wifi Board is first connected to the Flipper
    void ledStart() {
        pinMode(B_PIN, OUTPUT); // Set Blue Pin mode as output
        pinMode(G_PIN, OUTPUT); // Set Green Pin mode as output
        pinMode(R_PIN, OUTPUT); // Set Red Pin mode as output

        digitalWrite(B_PIN, OFF);
        digitalWrite(R_PIN, OFF);

        ledAction();
        ledAction();
        ledAction();
    }

    // Starting LED (Green only)
    void ledStatus() {
        digitalWrite(B_PIN, OFF);
        digitalWrite(R_PIN, OFF);
        digitalWrite(G_PIN, ON);
    }

    // Turn off all LEDs
    void ledOff() {
        digitalWrite(B_PIN, OFF);
        digitalWrite(G_PIN, OFF);
        digitalWrite(R_PIN, OFF);
    }

private:
    const char* settingsFilePath =
        "/flipper-http.json"; // Path to the settings file in the SPIFFS file system
    char loadedSSID[64] = {0}; // Variable to store SSID
    char loadedPassword[64] = {0}; // Variable to store password

    bool readSerialSettings(String receivedData, bool connectAfterSave);
};

//  Connect to Wifi using the loaded SSID and Password
bool FlipperHTTP::connectToWifi() {
    if(String(loadedSSID) == "" || String(loadedPassword) == "") {
        Serial.println("[ERROR] WiFi SSID or Password is empty.");
        return false;
    }

    WiFi.disconnect(true); // Ensure WiFi is disconnected before reconnecting
    WiFi.begin(loadedSSID, loadedPassword);

    int i = 0;
    while(!this->isConnectedToWifi() && i < 20) {
        delay(500);
        i++;
        Serial.print(".");
    }
    Serial.println(); // Move to next line after dots

    if(this->isConnectedToWifi()) {
        Serial.println("[SUCCESS] Successfully connected to Wifi.");
        return true;
    } else {
        Serial.println("[ERROR] Failed to connect to Wifi.");
        return false;
    }
}

// Save Wifi settings to SPIFFS
bool FlipperHTTP::saveWifiSettings(String jsonData) {
    File file = SPIFFS.open(settingsFilePath, FILE_WRITE);
    if(!file) {
        Serial.println("[ERROR] Failed to open file for writing.");
        return false;
    }

    file.print(jsonData);
    file.close();
    Serial.println("[SUCCESS] Settings saved to SPIFFS.");
    return true;
}

// Load Wifi settings from SPIFFS
bool FlipperHTTP::loadWifiSettings() {
    File file = SPIFFS.open(settingsFilePath, FILE_READ);
    if(!file) {
        Serial.println("[ERROR] Failed to open file for reading.");
        return "";
    }

    // Read the entire file content
    String fileContent = file.readString();
    file.close();

    return fileContent;
}

String FlipperHTTP::readSerialLine() {
    String receivedData = "";

    while(Serial.available() > 0) {
        char incomingChar = Serial.read();
        if(incomingChar == '\n') {
            break;
        }
        receivedData += incomingChar;
        delay(1); // Minimal delay to allow buffer to fill
    }

    receivedData.trim(); // Remove any leading/trailing whitespace

    return receivedData;
}

bool FlipperHTTP::readSerialSettings(String receivedData, bool connectAfterSave) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, receivedData);

    if(error) {
        Serial.print("[ERROR] Failed to parse JSON: ");
        Serial.println(error.c_str());
        return false;
    }

    // Extract values from JSON
    if(doc.containsKey("ssid") && doc.containsKey("password")) {
        strlcpy(loadedSSID, doc["ssid"], sizeof(loadedSSID));
        strlcpy(loadedPassword, doc["password"], sizeof(loadedPassword));
    } else {
        Serial.println("[ERROR] JSON does not contain ssid and password.");
        return false;
    }

    // Save to SPIFFS
    if(!this->saveWifiSettings(receivedData)) {
        Serial.println("[ERROR] Failed to save settings to file.");
        return false;
    }

    // Attempt to reconnect with new settings
    if(connectAfterSave && this->connectToWifi()) {
        Serial.println("[SUCCESS] Connected to the new Wifi network.");
    } else {
        Serial.println("[WARNING] Saved settings but failed to connect.");
    }

    return true;
}

String FlipperHTTP::get(String url) {
    WiFiClientSecure client;
    client.setInsecure(); // Bypass certificate validation

    HTTPClient http;
    String payload = "";

    if(http.begin(client, url)) {
        int httpCode = http.GET();

        if(httpCode > 0) {
            payload = http.getString();
            http.end();
            return payload;
        } else {
            Serial.print("[ERROR] GET Request Failed, error: ");
            Serial.println(http.errorToString(httpCode).c_str());
        }
        http.end();
    } else {
        Serial.println("[ERROR] Unable to connect to the server.");
    }

    // Clear serial buffer to avoid any residual data
    this->clearSerialBuffer();

    return payload;
}

String FlipperHTTP::get(
    String url,
    const char* headerKeys[],
    const char* headerValues[],
    int headerSize) {
    WiFiClientSecure client;
    client.setInsecure(); // Bypass certificate

    HTTPClient http;
    String payload = "";

    http.collectHeaders(headerKeys, headerSize);

    if(http.begin(client, url)) {
        for(int i = 0; i < headerSize; i++) {
            http.addHeader(headerKeys[i], headerValues[i]);
        }

        int httpCode = http.GET();

        if(httpCode > 0) {
            payload = http.getString();
            http.end();
            return payload;
        } else {
            Serial.print("[ERROR] GET Request Failed, error: ");
            Serial.println(http.errorToString(httpCode).c_str());
        }
        http.end();
    } else {
        Serial.println("[ERROR] Unable to connect to the server.");
    }

    // Clear serial buffer to avoid any residual data
    this->clearSerialBuffer();

    return payload;
}

String FlipperHTTP::delete_request(String url, String payload) {
    WiFiClientSecure client;
    client.setInsecure(); // Bypass certificate

    HTTPClient http;
    String response = "";

    if(http.begin(client, url)) {
        int httpCode = http.sendRequest("DELETE", payload);

        if(httpCode > 0) {
            response = http.getString();
            http.end();
            return response;
        } else {
            Serial.print("[ERROR] DELETE Request Failed, error: ");
            Serial.println(http.errorToString(httpCode).c_str());
        }
        http.end();
    }

    else {
        Serial.println("[ERROR] Unable to connect to the server.");
    }

    // Clear serial buffer to avoid any residual data
    this->clearSerialBuffer();

    return response;
}

String FlipperHTTP::delete_request(
    String url,
    String payload,
    const char* headerKeys[],
    const char* headerValues[],
    int headerSize) {
    WiFiClientSecure client;
    client.setInsecure(); // Bypass certificate

    HTTPClient http;
    String response = "";

    http.collectHeaders(headerKeys, headerSize);

    if(http.begin(client, url)) {
        for(int i = 0; i < headerSize; i++) {
            http.addHeader(headerKeys[i], headerValues[i]);
        }

        int httpCode = http.sendRequest("DELETE", payload);

        if(httpCode > 0) {
            response = http.getString();
            http.end();
            return response;
        } else {
            Serial.print("[ERROR] DELETE Request Failed, error: ");
            Serial.println(http.errorToString(httpCode).c_str());
        }
        http.end();
    }

    else {
        Serial.println("[ERROR] Unable to connect to the server.");
    }

    // Clear serial buffer to avoid any residual data
    this->clearSerialBuffer();

    return response;
}

String FlipperHTTP::post(
    String url,
    String payload,
    const char* headerKeys[],
    const char* headerValues[],
    int headerSize) {
    WiFiClientSecure client;
    client.setInsecure(); // Bypass certificate

    HTTPClient http;
    String response = "";

    http.collectHeaders(headerKeys, headerSize);

    if(http.begin(client, url)) {
        for(int i = 0; i < headerSize; i++) {
            http.addHeader(headerKeys[i], headerValues[i]);
        }

        int httpCode = http.POST(payload);

        if(httpCode > 0) {
            response = http.getString();
            http.end();
            return response;
        } else {
            Serial.print("[ERROR] POST Request Failed, error: ");
            Serial.println(http.errorToString(httpCode).c_str());
        }
        http.end();
    }

    else {
        Serial.println("[ERROR] Unable to connect to the server.");
    }

    // Clear serial buffer to avoid any residual data
    this->clearSerialBuffer();

    return response;
}

String FlipperHTTP::post(String url, String payload) {
    WiFiClientSecure client;
    client.setInsecure(); // Bypass certificate

    HTTPClient http;
    String response = "";

    if(http.begin(client, url)) {
        int httpCode = http.POST(payload);

        if(httpCode > 0) {
            response = http.getString();
            http.end();
            return response;
        } else {
            Serial.print("[ERROR] POST Request Failed, error: ");
            Serial.println(http.errorToString(httpCode).c_str());
        }
        http.end();
    }

    else {
        Serial.println("[ERROR] Unable to connect to the server.");
    }

    // Clear serial buffer to avoid any residual data
    this->clearSerialBuffer();

    return response;
}

String FlipperHTTP::put(
    String url,
    String payload,
    const char* headerKeys[],
    const char* headerValues[],
    int headerSize) {
    WiFiClientSecure client;
    client.setInsecure(); // Bypass certificate

    HTTPClient http;
    String response = "";

    http.collectHeaders(headerKeys, headerSize);

    if(http.begin(client, url)) {
        for(int i = 0; i < headerSize; i++) {
            http.addHeader(headerKeys[i], headerValues[i]);
        }

        int httpCode = http.PUT(payload);

        if(httpCode > 0) {
            response = http.getString();
            http.end();
            return response;
        } else {
            Serial.print("[ERROR] PUT Request Failed, error: ");
            Serial.println(http.errorToString(httpCode).c_str());
        }
        http.end();
    }

    else {
        Serial.println("[ERROR] Unable to connect to the server.");
    }

    // Clear serial buffer to avoid any residual data
    this->clearSerialBuffer();

    return response;
}

String FlipperHTTP::put(String url, String payload) {
    WiFiClientSecure client;
    client.setInsecure(); // Bypass certificate

    HTTPClient http;
    String response = "";

    if(http.begin(client, url)) {
        int httpCode = http.PUT(payload);

        if(httpCode > 0) {
            response = http.getString();
            http.end();
            return response;
        } else {
            Serial.print("[ERROR] PUT Request Failed, error: ");
            Serial.println(http.errorToString(httpCode).c_str());
        }
        http.end();
    }

    else {
        Serial.println("[ERROR] Unable to connect to the server.");
    }

    // Clear serial buffer to avoid any residual data
    this->clearSerialBuffer();

    return response;
}

void FlipperHTTP::setup() {
    Serial.begin(115200);
    // Initialize SPIFFS
    if(!SPIFFS.begin(true)) {
        Serial.println("[ERROR] SPIFFS initialization failed.");
        ESP.restart();
    }

    this->ledStart();
    Serial.flush();
}

void FlipperHTTP::loop() {
    // Check if there's incoming serial data
    if(Serial.available() > 0) {
        // Read the incoming serial data until newline
        String _data = this->readSerialLine();

        if(_data.length() == 0) {
            // No complete command received
            return;
        }

        this->ledStatus();

        // Ping/Pong to see if board/flipper is connected
        if(_data.startsWith("[PING]")) {
            Serial.println("[PONG]");
        }
        // Handle [WIFI/SAVE] command
        else if(_data.startsWith("[WIFI/SAVE]")) {
            // Extract JSON data by removing the command part
            String jsonData = _data.substring(strlen("[WIFI/SAVE]"));
            jsonData.trim(); // Remove any leading/trailing whitespace

            // Parse and save the settings
            if(this->readSerialSettings(jsonData, true)) {
                Serial.println("[SUCCESS] Wifi settings saved.");
            } else {
                Serial.println("[ERROR] Failed to save Wifi settings.");
            }
        }
        // Handle [WIFI/CONNECT] command
        else if(_data == "[WIFI/CONNECT]") {
            // Check if WiFi is already connected
            if(!this->isConnectedToWifi()) {
                // Attempt to connect to Wifi
                if(this->connectToWifi()) {
                    Serial.println("[SUCCESS] Connected to Wifi.");
                } else {
                    Serial.println("[ERROR] Failed to connect to Wifi.");
                }
            } else {
                Serial.println("[INFO] Already connected to Wifi.");
            }
        }
        // Handle [WIFI/DISCONNECT] command
        else if(_data == "[WIFI/DISCONNECT]") {
            WiFi.disconnect(true);
            Serial.println("[DISCONNECTED] Wifi has been disconnected.");
        }
        // Handle [GET] command
        else if(_data.startsWith("[GET]")) {
            if(!this->isConnectedToWifi() && !this->connectToWifi()) {
                Serial.println("[ERROR] Not connected to Wifi. Failed to reconnect.");
                this->ledOff();
                return;
            }
            // Extract URL by removing the command part
            String url = _data.substring(strlen("[GET]"));
            url.trim();

            // GET request
            String getData = this->get(url);
            if(getData != "") {
                Serial.println("[GET/SUCCESS] GET request successful.");
                Serial.println(getData);
                Serial.flush();
                Serial.println();
                Serial.println("[GET/END]");
            } else {
                Serial.println("[ERROR] GET request failed or returned empty data.");
            }
        }
        // Handle [GET/HTTP] command
        else if(_data.startsWith("[GET/HTTP]")) {
            if(!this->isConnectedToWifi() && !this->connectToWifi()) {
                Serial.println("[ERROR] Not connected to Wifi. Failed to reconnect.");
                this->ledOff();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[GET/HTTP]"));
            jsonData.trim();

            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, jsonData);

            if(error) {
                Serial.print("[ERROR] Failed to parse JSON.");
                this->ledOff();
                return;
            }

            // Extract values from JSON
            if(!doc.containsKey("url")) {
                Serial.println("[ERROR] JSON does not contain url.");
                this->ledOff();
                return;
            }
            String url = doc["url"];

            // Extract headers if available
            const char* headerKeys[10];
            const char* headerValues[10];
            int headerSize = 0;

            if(doc.containsKey("headers")) {
                JsonObject headers = doc["headers"];
                for(JsonPair header : headers) {
                    headerKeys[headerSize] = header.key().c_str();
                    headerValues[headerSize] = header.value();
                    headerSize++;
                }
            }

            // GET request
            String getData = this->get(url, headerKeys, headerValues, headerSize);
            if(getData != "") {
                Serial.println("[GET/SUCCESS] GET request successful.");
                Serial.println(getData);
                Serial.flush();
                Serial.println();
                Serial.println("[GET/END]");
            } else {
                Serial.println("[ERROR] GET request failed or returned empty data.");
            }
        }
        // Handle [POST/HTTP] command
        else if(_data.startsWith("[POST/HTTP]")) {
            if(!this->isConnectedToWifi() && !this->connectToWifi()) {
                Serial.println("[ERROR] Not connected to Wifi. Failed to reconnect.");
                this->ledOff();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[POST/HTTP]"));
            jsonData.trim();

            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, jsonData);

            if(error) {
                Serial.print("[ERROR] Failed to parse JSON.");
                this->ledOff();
                return;
            }

            // Extract values from JSON
            if(!doc.containsKey("url") || !doc.containsKey("payload")) {
                Serial.println("[ERROR] JSON does not contain url or payload.");
                this->ledOff();
                return;
            }
            String url = doc["url"];
            String payload = doc["payload"];

            // Extract headers if available
            const char* headerKeys[10];
            const char* headerValues[10];
            int headerSize = 0;

            if(doc.containsKey("headers")) {
                JsonObject headers = doc["headers"];
                for(JsonPair header : headers) {
                    headerKeys[headerSize] = header.key().c_str();
                    headerValues[headerSize] = header.value();
                    headerSize++;
                }
            }

            // POST request
            String postData = this->post(url, payload, headerKeys, headerValues, headerSize);
            if(postData != "") {
                Serial.println("[POST/SUCCESS] POST request successful.");
                Serial.println(postData);
                Serial.flush();
                Serial.println();
                Serial.println("[POST/END]");
            } else {
                Serial.println("[ERROR] POST request failed or returned empty data.");
            }
        }
        // Handle [PUT/HTTP] command
        else if(_data.startsWith("[PUT/HTTP]")) {
            if(!this->isConnectedToWifi() && !this->connectToWifi()) {
                Serial.println("[ERROR] Not connected to Wifi. Failed to reconnect.");
                this->ledOff();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[PUT/HTTP]"));
            jsonData.trim();

            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, jsonData);

            if(error) {
                Serial.print("[ERROR] Failed to parse JSON.");
                this->ledOff();
                return;
            }

            // Extract values from JSON
            if(!doc.containsKey("url") || !doc.containsKey("payload")) {
                Serial.println("[ERROR] JSON does not contain url or payload.");
                this->ledOff();
                return;
            }
            String url = doc["url"];
            String payload = doc["payload"];

            // Extract headers if available
            const char* headerKeys[10];
            const char* headerValues[10];
            int headerSize = 0;

            if(doc.containsKey("headers")) {
                JsonObject headers = doc["headers"];
                for(JsonPair header : headers) {
                    headerKeys[headerSize] = header.key().c_str();
                    headerValues[headerSize] = header.value();
                    headerSize++;
                }
            }

            // PUT request
            String putData = this->put(url, payload, headerKeys, headerValues, headerSize);
            if(putData != "") {
                Serial.println("[PUT/SUCCESS] PUT request successful.");
                Serial.println(putData);
                Serial.flush();
                Serial.println();
                Serial.println("[PUT/END]");
            } else {
                Serial.println("[ERROR] PUT request failed or returned empty data.");
            }
        }
        // Handle [DELETE/HTTP] command
        else if(_data.startsWith("[DELETE/HTTP]")) {
            if(!this->isConnectedToWifi() && !this->connectToWifi()) {
                Serial.println("[ERROR] Not connected to Wifi. Failed to reconnect.");
                this->ledOff();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[DELETE/HTTP]"));
            jsonData.trim();

            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, jsonData);

            if(error) {
                Serial.print("[ERROR] Failed to parse JSON.");
                this->ledOff();
                return;
            }

            // Extract values from JSON
            if(!doc.containsKey("url") || !doc.containsKey("payload")) {
                Serial.println("[ERROR] JSON does not contain url or payload.");
                this->ledOff();
                return;
            }
            String url = doc["url"];
            String payload = doc["payload"];

            // Extract headers if available
            const char* headerKeys[10];
            const char* headerValues[10];
            int headerSize = 0;

            if(doc.containsKey("headers")) {
                JsonObject headers = doc["headers"];
                for(JsonPair header : headers) {
                    headerKeys[headerSize] = header.key().c_str();
                    headerValues[headerSize] = header.value();
                    headerSize++;
                }
            }

            // DELETE request
            String deleteData =
                this->delete_request(url, payload, headerKeys, headerValues, headerSize);
            if(deleteData != "") {
                Serial.println("[DELETE/SUCCESS] DELETE request successful.");
                Serial.println(deleteData);
                Serial.flush();
                Serial.println();
                Serial.println("[DELETE/END]");
            } else {
                Serial.println("[ERROR] DELETE request failed or returned empty data.");
            }
        }

        this->ledOff();
    }
}
