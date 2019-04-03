//-------------------------------------------------------------------------------
// Licence:
// Copyright (c) 2019 Luzzi Valerio 
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
//
// Name:         SpeedButton.ino
// Purpose:      A button for speed climb
//
// Author:      Luzzi Valerio
//
// Created:     28/03/2019
//-------------------------------------------------------------------------------

#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include "arduino_secrets.h" 


const int BUTTON_PIN = 4;   
int POLLING_INTERVAL = 1000;


enum state {

  READY,
  IDLE,
  ARMED,
  RUNNING,
};

state STATE;

//------------ WiFI -----------------------------------------------------------------
int status = WL_IDLE_STATUS;
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;        // your network password (use for WPA, or use as key for WEP)
unsigned int localPort  = 10999;      // local port to listen on
unsigned int remotePort = 11000;
char cmd[64]; //buffer command to broadcast  

WiFiUDP Udp;
IPAddress broadcastIp(255,255,255,255);

void setup() {

  // initialize the button pin as a input:
  pinMode(BUTTON_PIN, INPUT);
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);

  // check for the presence of the shield:
  while (WiFi.status() == WL_NO_SHIELD) {
      Serial.println("WiFi shield not present");
      delay(1000);
  }

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(2000);
  }
  Serial.println("Connected to wifi");
  printWiFiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  Udp.begin(localPort);

 
  broadcast("ready:");
  STATE=RUNNING;
}

void loop() {
      
      if (STATE==RUNNING && digitalRead(BUTTON_PIN)==HIGH){
          
          broadcast("stop:");   
      }
      
      delay(1);
}

void broadcast(char* text){

    Udp.beginPacket(broadcastIp, remotePort);
    Udp.write(text);
    Udp.endPacket();
    Serial.println(text);
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
