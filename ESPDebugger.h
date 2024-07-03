#ifndef ESPDebugger_h
#define ESPDebugger_h

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <RemoteDebug.h>
#include <ArduinoOTA.h>
#include <time.h>

class ESPDebugger {
  public:
    ESPDebugger(int webPort = 80);
    void begin(const char* ssid, const char* password, const char* hostname = "ESP32");
    void handle();
    void print(String message);
    void setMaxLogSize(int size);

  private:
    WebServer server;
    RemoteDebug Debug;
    String debugLog;
    int maxLogSize;
    void handleRoot();
    void handleDebug();
    void handleGetLog();
    String getTimeString();
};

#endif