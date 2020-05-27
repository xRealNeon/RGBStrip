#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>
#include <RGB_LED.h>

#define REDPIN D1
#define GREENPIN D2
#define BLUEPIN D3

RGB_LED LED(REDPIN, GREENPIN, BLUEPIN);
WiFiManager wifiManager;
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

const double VW_VERSION = 1.0;
const int led = 13;
int fadeSpeed = 3000;

void setup(void) {
  pinMode(D8, INPUT);
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  //WiFi.mode(WIFI_STA);
  //WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("LedStrip v" + String(VW_VERSION));
  Serial.println("Connecting to WIFI");

  // Wait for connection
  //while (WiFi.status() != WL_CONNECTED) {
  //  delay(500);
  //  Serial.print(".");
  //}
  //Serial.println("");
  //Serial.print("Connected to ");
  //Serial.println(ssid);
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());

  wifiManager.autoConnect();

  if (MDNS.begin("TestESP")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/setled", []() {
    LED.fadeTo(getArgValue("r"), getArgValue("g"), getArgValue("b"), 1000);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", "{\"status\":\"success\",\"action\":\"setled\"}");
  });

  server.on("/ledoff", []() {
    LED.fadeTo(0, 0, 0, 1000);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", "{\"status\":\"success\",\"action\":\"ledoff\"}");
  });

  server.on("/rainbow", []() {
    LED.setFunction(Fade);
    LED.setSpeed(fadeSpeed);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", "{\"status\":\"success\",\"action\":\"rainbow\"}");
  });

  server.on("/setspeed", []() {
    LED.setSpeed(getArgValue("speed"));
    fadeSpeed = getArgValue("speed");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", "{\"status\":\"success\",\"action\":\"setspeed\"}");
  });

  server.on("/getdata", []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", "{\"status\":\"success\",\"action\":\"getdata\",\"function\":" + String(LED.getFunction()) + ",\"speed\":" + String(LED.getSpeed()) + ",\"r\":" + String(LED.getCurrentRValue()) + ",\"g\":" + String(LED.getCurrentGValue()) + ",\"b\":" + String(LED.getCurrentBValue()) + "}");
  });

  server.on("/ping", []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", "{\"status\":\"success\",\"action\":\"ping\"}");
  });

  httpUpdater.setup(&server, "firmware", "admin", "update");
  server.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", "192.168.178.78");
  Serial.println("HTTP server started");

  server.onNotFound(handleNotFound);


  LED.setFunction(Solid);
}

void loop(void) {
  LED.run();
  server.handleClient();
  MDNS.update();

  if ( digitalRead(D8) == HIGH ) {
    WiFiManager wifiManager;
    WiFi.mode(WIFI_STA);
    wifiManager.startConfigPortal("OnDemandAP");
    Serial.println("connected...yeey :)");
  }
}

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "LedStrip v" + String(VW_VERSION));
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

int getArgValue(String name)
{
  for (uint8_t i = 0; i < server.args(); i++)
    if (server.argName(i) == name)
      return server.arg(i).toInt();
  return -1;
}
