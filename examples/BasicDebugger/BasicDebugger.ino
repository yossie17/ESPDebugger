#include "ESPDebugger.h"
ESPDebugger debugger;

void setup() {
  debugger.begin("SSID", "Password");
// Your existing setup code

}

void loop() {
  debugger.handle();
// Your existing loop code
  debugger.print("Loop running...");

}
