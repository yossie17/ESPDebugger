#include "ESPDebugger.h"

ESPDebugger::ESPDebugger(int webPort) : server(webPort), maxLogSize(5000) {}

void ESPDebugger::begin(const char* ssid, const char* password, const char* hostname) {
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
  Debug.begin(hostname, Debug.VERBOSE);
  Debug.setSerialEnabled(true);
  Debug.setResetCmdEnabled(true);
  Serial.println("Telnet server started");

  // OTA Setup
  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA is ready");
}

void ESPDebugger::handle() {
  server.handleClient();
  Debug.handle();
  ArduinoOTA.handle();
}

void ESPDebugger::print(String message) {
  String timestampedMessage = getTimeString() + " - " + message;
  Serial.println(timestampedMessage);
  Debug.println(timestampedMessage);
  debugLog += timestampedMessage + "\n";
  if (debugLog.length() > maxLogSize) {
    int cutoff = debugLog.indexOf("\n", debugLog.length() - maxLogSize);
    if (cutoff != -1) {
      debugLog = debugLog.substring(cutoff + 1);
    }
  }
}

void ESPDebugger::setMaxLogSize(int size) {
  maxLogSize = size;
}

String ESPDebugger::getTimeString() {
  time_t now = time(nullptr);
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  char timeString[30];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeString);
}

void ESPDebugger::handleRoot() {
  String html = "<html><body>";
  html += "<h1>ESP32 Debug Server</h1>";
  html += "<a href='/debug' target='_blank'>View Debug Log</a>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void ESPDebugger::handleDebug() {
  String html = R"(
    <html>
    <head>
      <title>ESP32 Debug Log</title>
      <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 20px; }
        h1 { color: #333; }
        #logContainer { 
          border: 1px solid #ccc; 
          padding: 10px; 
          height: 400px; 
          overflow-y: scroll;
          background-color: #f9f9f9;
        }
        pre { margin: 0; white-space: pre-wrap; word-wrap: break-word; }
      </style>
    </head>
    <body>
      <h1>ESP32 Debug Log</h1>
      <p>Last updated: <span id="lastUpdated"></span></p>
      <div id='logContainer'>
        <pre id='logContent'></pre>
      </div>
      <script>
        function updateLog() {
          var xhr = new XMLHttpRequest();
          xhr.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              document.getElementById('logContent').innerHTML = this.responseText;
              document.getElementById('lastUpdated').textContent = new Date().toLocaleString();
              var logContainer = document.getElementById('logContainer');
              logContainer.scrollTop = logContainer.scrollHeight;
            }
          };
          xhr.open('GET', '/getLog', true);
          xhr.send();
        }
        setInterval(updateLog, 1000);
        updateLog();
      </script>
    </body>
    </html>
  )";
  server.send(200, "text/html", html);
}

void ESPDebugger::handleGetLog() {
  server.send(200, "text/plain", debugLog);
}