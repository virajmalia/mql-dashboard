#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

#include <WiFi.h>
#include <PubSubClient.h>

// WiFi and MQTT Configuration
const char* ssid = "ssid";
const char* password = "pass";
const char* mqttServer = "localhost";
const int mqttPort = 1883;

const char* bal_topic   = "mt5/balance";
const char* eq_topic    = "mt5/equity";
const char* margin_topic = "mt5/margin";
const char* profit_topic = "mt5/profit";

String balanceString = "";
String equityString = "";
String marginString = "";
String profitString = "";

// Financial data structure
struct FinancialData {
    float balance;
    float equity;
    float margin;
    float profit;
    
    FinancialData() : balance(0.0f), equity(0.0f), margin(0.0f), profit(0.0f) {}
    FinancialData(float x) : balance(x), equity(x), margin(x), profit(x) {}
} finData, prevFinData(1);

// Declaration
void updateMetrics(const int updateParam);

void wifiSetup() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.println("Callback triggered.");
    Serial.println(topic);
    
    bool topic_is_bal = !strncmp(topic, bal_topic, length);
    bool topic_is_eq = !strncmp(topic, eq_topic, length);
    bool topic_is_margin = !strncmp(topic, margin_topic, length);
    bool topic_is_profit = !strncmp(topic, profit_topic, length);
    
    if (topic_is_bal) {
        balanceString = "";
        for (int i = 0; i < length; i++) {
            balanceString += (char)payload[i];
        }
        finData.balance = balanceString.toFloat();
        updateMetrics(1);
    }
    if (topic_is_eq) {
        equityString = "";
        for (int i = 0; i < length; i++) {
            equityString += (char)payload[i];
        }
        finData.equity = equityString.toFloat();
        updateMetrics(2);
    }
    if (topic_is_margin) {
        marginString = "";
        for (int i = 0; i < length; i++) {
            marginString += (char)payload[i];
        }
        finData.margin = marginString.toFloat();
        updateMetrics(3);
    }
    if (topic_is_profit) {
        profitString = "";
        for (int i = 0; i < length; i++) {
            profitString += (char)payload[i];
        }
        finData.profit = profitString.toFloat();
        updateMetrics(4);
    }
}

void reconnect(PubSubClient& client) {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32Client")) {
            Serial.println("connected");
            client.subscribe(eq_topic);
            client.subscribe(bal_topic);
            client.subscribe(margin_topic);
            client.subscribe(profit_topic);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

#endif  // MQTT_CONFIG_H
