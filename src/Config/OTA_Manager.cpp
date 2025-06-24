#include "Config/OTA_Manager.h"
#include "StateMachine/WebSocket_Manager.h" // For OTA status updates

//* ************************************************************************
//* ************************ NETWORK CONFIGURATION **********************
//* ************************************************************************
const char* WIFI_SSID = "Everwood";
const char* WIFI_PASSWORD = "Everwood-Staff";

//* ************************************************************************
//* ************************ WIFI FUNCTIONS *****************************
//* ************************************************************************

void initWiFi() {
  SerialWS_print("Connecting to WiFi: ");
  SerialWS_println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) { // Try for 10 seconds
    delay(500);
    SerialWS_print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    SerialWS_println("\nWiFi connected!");
    SerialWS_print("IP Address: ");
    SerialWS_println(WiFi.localIP().toString());
  } else {
    SerialWS_println("\nFailed to connect to WiFi. OTA and WebSocket might not work.");
  }
}

void displayIP(){
    SerialWS_println("\n=== Network Information ===");
    SerialWS_print("SSID: ");
    SerialWS_println(WIFI_SSID);
    SerialWS_print("IP Address: ");
    SerialWS_println(WiFi.localIP().toString());
    SerialWS_print("MAC Address: ");
    SerialWS_println(WiFi.macAddress());
    SerialWS_print("Signal Strength (RSSI): ");
    SerialWS_print(WiFi.RSSI());
    SerialWS_println(" dBm");
    SerialWS_println("===========================");
}

//* ************************************************************************
//* ************************ OTA FUNCTIONS ********************************
//* ************************************************************************

void initOTA() {
  if (WiFi.status() != WL_CONNECTED) {
    SerialWS_println("OTA Init Failed: WiFi not connected.");
    return;
  }

  ArduinoOTA.setHostname("esp32-s3-ota"); // Optional: Set a hostname

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else { // U_SPIFFS
        type = "filesystem";
      }
      SerialWS_println("OTA Start updating " + type);
      sendOTAStartEvent(type); // Send event to WebSocket clients
    })
    .onEnd([]() {
      SerialWS_println("\nOTA End");
      sendOTAEndEvent(); // Send event to WebSocket clients
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      SerialWS_printf("OTA Progress: %u%%\r", (progress / (total / 100)));
      sendOTAProgress(progress, total); // Send progress to WebSocket clients
    })
    .onError([](ota_error_t error) {
      SerialWS_printf("OTA Error[%u]: ", error);
      String errorMessage;
      if (error == OTA_AUTH_ERROR) errorMessage = "Auth Failed";
      else if (error == OTA_BEGIN_ERROR) errorMessage = "Begin Failed";
      else if (error == OTA_CONNECT_ERROR) errorMessage = "Connect Failed";
      else if (error == OTA_RECEIVE_ERROR) errorMessage = "Receive Failed";
      else if (error == OTA_END_ERROR) errorMessage = "End Failed";
      else errorMessage = "Unknown Error";
      SerialWS_println(errorMessage);
      sendOTAErrorEvent(errorMessage); // Send error to WebSocket clients
    });

  ArduinoOTA.begin();
  SerialWS_println("OTA Initialized");
  displayIP(); // Display IP after OTA init as well
}

void handleOTA() {
  if (WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.handle();
  }
} 