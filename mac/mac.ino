#include <WiFi.h>

void setup() {
  Serial.begin(115200);

  // 1. Force a disconnect (just in case)
  WiFi.disconnect(true); 

  // 2. Set the mode (WIFI_STA is correct for ESP-NOW)
  WiFi.mode(WIFI_STA); 
  delay(100); // Give it a moment to initialize

  // 3. Print the MAC address
  Serial.print("\n\n--- Actual Receiver MAC Address ---\n");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("-----------------------------------\n\n");
}

void loop() {
  // Nothing else is needed
}