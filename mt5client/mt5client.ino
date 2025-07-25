#include <NTPClient.h>
#include "mqtt_config.h"
#include "display.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
    Serial.begin(115200);
    
    // Initialize WiFi
    wifiSetup();
    timeClient.begin();
    timeClient.setTimeOffset(-25200);
    
    // Initialize display
    display.init();
    display.setRotation(1);
    
    // Calculate metric positions
    calculateMetricPositions();
    
    // Draw initial display
    drawInitialDisplay();
    
    // Setup MQTT
    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback); // callback performs screen updates
    
    Serial.println("Setup complete.");
}

void loop() {
    // Keep MQTT connection alive
    if (!client.connected()) {
        reconnect(client);
    }
    client.loop();
    delay(100);
}
