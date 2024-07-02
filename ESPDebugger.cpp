#include "ESPDebugger.h"

ESPDebugger::ESPDebugger(int webPort) : server(webPort), maxLogSize(5000) {}

void ESPDebugger::begin(const char* ssid, const char* password) {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < 1000000000) {
    delay(100);
    Serial.println("Waiting for time sync...");
  }

  // Web Server Setup
  server.on("/", std::bind(&ESPDebugger::handleRoot, this));
  server.on("/debug", std::bind(&ESPDebugger::handleDebug, this));
  server.on("/getLog", std::bind(&ESPDebugger::handleGetLog, this));
  server.begin();
  Serial.println("Web server started");

  // RemoteDebug Setup
  Debug.begin("ESP32"); // Initialize the library with your module name
  Debug.setResetCmdEnabled(true); // Enable the reset command
  Serial.println("Telnet server started");
}

void ESPDebugger::handle() {
  server.handleClient();
  Debug.handle();
}

void ESPDebugger::print(String message) {
  String timestampedMessage = getTimeString() + " - " + message;
  Serial.println(timestampedMessage);
  Debug.println(timestampedMessage); // Send to RemoteDebug
  debugLog += timestampedMessage + "\n";
  if (debugLog.length() > maxLogSize) {
    int cutoff = debugLog.indexOf("\n", debugLog.length() - maxLogSize);
    if (cutoff != -1) {
      debugLog = debugLog.substring(cutoff + 1);
    }
  }
}

// ... (rest of the methods remain the same as in the previous version)