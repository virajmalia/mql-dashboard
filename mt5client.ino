#include <WiFi.h>
#include <NTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSansOblique18pt7b.h>
#include <Ubuntu_Regular20pt7b.h>
#include "GxEPD2_display_selection.h"
#include "GxEPD2_display_selection_added.h"

// Configure display
GxEPD2_3C<GxEPD2_420c_Z21, GxEPD2_420c_Z21::HEIGHT>
    display(
        GxEPD2_420c_Z21(/*CS=*/ 5, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)
        );

// Configure WiFi
const char* ssid = "my_ssid";
const char* password = "my_pass";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// MQTT broker details
const char* mqttServer = "<IP address>";
const int mqttPort = 1883;
const char* bal_topic = "mt5/balance";
const char* eq_topic = "mt5/equity";

WiFiClient espClient;
PubSubClient client(espClient);

String balanceString = "";
String equityString = "";

void connectWifi()
{
  Serial.begin(115200);
  delay(10);

  // Connect to WiFi
  Serial.println("\nConnecting to WiFi");
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  timeClient.begin();

  // Print local IP address
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void setup_fonts()
{
  display.setFont(&Ubuntu_Regular20pt7b);
  display.setTextSize(1);
  display.setTextColor(GxEPD_BLACK);
}

String get_time_str(int hours, int mins)
{
  String hours_str, mins_str, suffix, text;

  if(mins < 10){
    mins_str = "0";
  }
  else{
    mins_str = "";
  }
  if(hours >= 12){
    suffix = "PM";
    hours_str = String(hours-12);
  }
  else{
    suffix = "AM";
    hours_str = String(hours);
  }
  text = hours_str + ":" + mins_str + String(mins) + " " + suffix;
  return text;
}

String get_current_time_str()
{
  do{
    timeClient.update();
  } while(timeClient.getSeconds() != 0);
  int hours = timeClient.getHours();
  int mins = timeClient.getMinutes();

  String text = get_time_str(hours, mins);
  return text;
}

void init_display()
{
  display.init();
  display.fillScreen(GxEPD_WHITE);
  setup_fonts();
  display.setCursor(0, 100);
  //display.print("Loading...");
  display.display();
}

void callback(char* topic, byte* payload, unsigned int length) {
  bool topic_is_bal = !strncmp(topic, bal_topic, length);
  bool topic_is_eq = !strncmp(topic, eq_topic, length);
  bool topic_is_pos = !strncmp(topic, pos_topic, length);
  for (int i = 0; i < length; i++) {
    if (topic_is_bal)
      balanceString += (char)payload[i];
    else if (topic_is_eq)
      equityString += (char)payload[i];
  }

  if (topic_is_eq){
    //fullRefresh();
    displayContents("Balance: $"+equityString, 10, "top-left", GxEPD_BLACK);
  }
  else if (topic_is_bal){
    float balance = balanceString.toFloat();
    float equity = equityString.toFloat();
    float curr_pnl = equity - balance;
    displayContents("P&L: $"+String(curr_pnl), 10, "center-right", GxEPD_RED);
    balanceString = "";
    equityString = "";
  }
  delay(100);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      // Subscribe to the desired topic(s)
      client.subscribe(eq_topic);
      client.subscribe(bal_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void fullRefresh(){
  display.fillScreen(GxEPD_WHITE);  // clear previous contents
  display.display();
}

void displayContents(String data_str, uint16_t margin, String display_slot, uint16_t text_color){
  if (data_str == "")
    return;
  //String text = get_current_time_str();
  // Adafruit_GFX has a handy method getTextBounds() to determine the boundary box for a text for the actual font
  int16_t tbx, tby; uint16_t tbw, tbh; // boundary box window
  display.getTextBounds(data_str, 0, 0, &tbx, &tby, &tbw, &tbh); // it works for origin 0, 0, fortunately (negative tby!)
  // center bounding box by transposition of origin:
  //uint16_t x = ((display.width() - tbw) / 2) - tbx;
  //uint16_t y = ((display.height() - tbh) / 2) - tby;

  uint16_t x = 0;
  uint16_t y = 0;
  if (display_slot == "center-right"){
    x = display.width()/2;
    y = 2*(tbh + margin);
  }
  else if (display_slot == "top-right"){
    x = display.width()/2;
    y = tbh + margin;
  }
  else if (display_slot == "top-left"){
    x = margin;
    y = tbh + margin;
  }

  display.fillRect(x, y-tbh, tbw+3, tbh+3, GxEPD_WHITE);
  display.setCursor(x, y);
  display.setTextColor(text_color);
  display.print(data_str);
  //display.display();
  display.displayWindow(x, y-tbh, tbw+3, tbh+3); // Update only the area where the data was drawn
}

void setup() {
  connectWifi();
  timeClient.setTimeOffset(-25200);
  init_display();

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(100);
}
