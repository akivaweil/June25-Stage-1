#ifndef WEBSOCKET_MANAGER_H
#define WEBSOCKET_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

//* ************************************************************************
//* ************************ WEBSOCKET CONFIGURATION ********************
//* ************************************************************************

// Web server port
#define WEB_SERVER_PORT 80

// Message buffer configuration
#define MESSAGE_BUFFER_SIZE 100        // Number of recent messages to store
#define HEARTBEAT_INTERVAL 30000       // Heartbeat interval in milliseconds
#define CONNECTION_TIMEOUT 60000       // Connection timeout in milliseconds

//* ************************************************************************
//* ************************ FUNCTION DECLARATIONS **********************
//* ************************************************************************

// WebSocket server setup and management functions
void initWebSocket();
void handleWebSocket();
void sendToWebSocket(const String& message);
void sendBufferedMessagesToClient(AsyncWebSocketClient *client);
void addToMessageBuffer(const String& message);
void sendHeartbeat();
void checkConnectionHealth();

// Motor movement WebSocket control functions
void pauseWebSocketTransmission();
void resumeWebSocketTransmission();
bool isWebSocketPaused();
void addToPausedBuffer(const String& message);

// Custom Serial functions that output to both Serial and WebSocket
// String versions
void SerialWS_print(const String& message);
void SerialWS_println(const String& message);
void SerialWS_printf(const char* format, ...);

// Character versions
void SerialWS_print(char c);
void SerialWS_println(char c);
void SerialWS_print(const char* str);
void SerialWS_println(const char* str);

// Integer versions
void SerialWS_print(int value);
void SerialWS_println(int value);
void SerialWS_print(unsigned int value);
void SerialWS_println(unsigned int value);
void SerialWS_print(long value);
void SerialWS_println(long value);
void SerialWS_print(unsigned long value);
void SerialWS_println(unsigned long value);

// Float versions
void SerialWS_print(float value);
void SerialWS_println(float value);
void SerialWS_print(double value);
void SerialWS_println(double value);
void SerialWS_print(float value, int digits);
void SerialWS_println(float value, int digits);
void SerialWS_print(double value, int digits);
void SerialWS_println(double value, int digits);

// Boolean version
void SerialWS_print(bool value);
void SerialWS_println(bool value);

// Hex versions
void SerialWS_print(int value, int base);
void SerialWS_println(int value, int base);
void SerialWS_print(unsigned int value, int base);
void SerialWS_println(unsigned int value, int base);
void SerialWS_print(long value, int base);
void SerialWS_println(long value, int base);
void SerialWS_print(unsigned long value, int base);
void SerialWS_println(unsigned long value, int base);

// Web server functions
void initWebServer(); // Typically called by initWebSocket
void handleWebServer(); // Typically called by handleWebSocket

// OTA status update functions
void sendOTAProgress(unsigned int progress, unsigned int total);
void sendOTAStartEvent(const String& updateType);
void sendOTAEndEvent();
void sendOTAErrorEvent(const String& errorMessage);


#endif 