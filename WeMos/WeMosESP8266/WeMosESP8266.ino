#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>




#ifndef STASSID
#define STASSID "Casa"
#define STAPSK  "annaanna"
#define APSSID "SpeedClimbBase"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

unsigned int wifi_attemps=0;
unsigned int APMODE=WIFI_STA;
unsigned int localPort  = 10999;      // local port to listen on
unsigned int remotePort = 11000;      // remote display port

ESP8266WebServer server(80);
MDNSResponder mdns;

void handleRoot() {
  digitalWrite(LED_BUILTIN, LOW);
  server.send(200, "text/plain", "hello WORLD esp8266!");
  digitalWrite(LED_BUILTIN, HIGH);
}

void handleNotFound() {
  digitalWrite(LED_BUILTIN, LOW);
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
  digitalWrite(LED_BUILTIN, HIGH);
}

void WIFI_setup(void){

  if (APMODE==WIFI_STA){
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      // Wait for connection
      while (WiFi.status() != WL_CONNECTED && wifi_attemps<20) {
          wifi_attemps++;
          delay(500);
          Serial.print(".");
      }
      if (WiFi.status() == WL_CONNECTED){
          Serial.print("Access Point:");
          Serial.print(STASSID);
          Serial.print("\nIP address:\t");
          Serial.println(WiFi.localIP());
      }else{
          APMODE=WIFI_AP;
      }

      while (!mdns.begin("esp8266")) {        
          Serial.print(".");
      }
      Serial.println("mDNS responder started");
  }
  
  if (APMODE==WIFI_AP){
      WiFi.mode(WIFI_AP);
      WiFi.softAP(APSSID);
      Serial.print("Access Point:");
      Serial.print(APSSID);
      Serial.print("IP address:\t");
      Serial.println(WiFi.softAPIP()); 
  }
  
}//setupWIFI


void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(9600);
  WIFI_setup();

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.on("/on", []() {
    digitalWrite(LED_BUILTIN, LOW);
    server.send(200, "text/plain", "led ON");
  });

  server.on("/off", []() {
    digitalWrite(LED_BUILTIN, HIGH);
    server.send(200, "text/plain", "led OFF");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  // Add service to MDNS-SD
  mdns.addService("http", "tcp", 80);
}

void loop(void) {
  mdns.update();

  server.handleClient();

}
