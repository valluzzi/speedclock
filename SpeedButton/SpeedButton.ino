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
//#include <WiFi101.h> //MKR1000
#include <WiFiNINA.h>  //MKR1010
#include <WiFiUdp.h>
#include "arduino_secrets.h" 


const int BUTTON_PIN = 2;   
int MIN_POLLING_INTERVAL = 1;
int MAX_POLLING_INTERVAL = 5000;
int POLLING_INTERVAL = 1000;
unsigned long mills_from_last_imalive =0;
unsigned long mills_active =0;

enum state {

  IDLE,
  RUNNING,
  SLEEP, 
  DEAD
};

state STATE;

//------------ WiFI -----------------------------------------------------------------
int status = WL_IDLE_STATUS;
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;        // your network password (use for WPA, or use as key for WEP)
unsigned int LOCAL_PORT  = 10999;      // local port to listen on
unsigned int REMOTE_PORT = 11000;
char cmd[64]; //buffer command to broadcast  
char buff[255]; //buffer command to broadcast 
int packetsize =0;

WiFiUDP Udp;
IPAddress broadcastIp(255,255,255,255);

void setup() {

  //WiFi.lowPowerMode();
  
  // initialize the button pin as a input:
  pinMode(BUTTON_PIN, INPUT);
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);

  // check for the presence of the shield:
  while (WiFi.status() == WL_NO_SHIELD) {
      Serial.println("WiFi shield not present");
      delay(POLLING_INTERVAL);
  }

  WiFiConnect();
  
  STATE=IDLE;
}

void loop() {

      if (STATE==IDLE || STATE==SLEEP){
          packetsize = receivePacket(buff);

          if (packetsize && strcmp(buff,"armed:")==0){
              Serial.println(buff);
              POLLING_INTERVAL = MIN_POLLING_INTERVAL*100;
              STATE = IDLE;
              mills_active = 0;
              WiFi.noLowPowerMode();
          }

          if (packetsize && strcmp(buff,"start:")==0){
              Serial.println(buff);
              POLLING_INTERVAL = MIN_POLLING_INTERVAL;
              STATE = RUNNING;
              mills_active = 0;
              WiFi.noLowPowerMode();
          }
      }

      //STOP - BUTTON PRESSED - SLOW DOWN
      if (STATE==RUNNING && digitalRead(BUTTON_PIN)==HIGH){
          Serial.println("stop:");
          repeated_broadcast("stop:"); 
          POLLING_INTERVAL = 1000; 
          STATE = IDLE; 
      }

      //SLEEP MODE AFTER 5-10 minutes
      if ((STATE==IDLE || STATE==RUNNING) && mills_active>30000)  {
          
          Serial.println("SLEEP MODE");
          POLLING_INTERVAL = MAX_POLLING_INTERVAL; 
          STATE = SLEEP; 
          WiFi.lowPowerMode();
      }

      if ((STATE==SLEEP) && mills_active>60000)  {
          Serial.println("DEAD MODE");
          POLLING_INTERVAL = MAX_POLLING_INTERVAL; 
          STATE = DEAD; 
          WiFi.end();
          status = WL_IDLE_STATUS;
      }
      //WAKE UP BY BUTTON FROm SLEEP MODE
      if (STATE==SLEEP && digitalRead(BUTTON_PIN)==HIGH){
          Serial.println("Wake up!");
          POLLING_INTERVAL = 1000; 
          STATE = IDLE; 
          mills_active=0;
          WiFi.noLowPowerMode();
      }
      //WAKE UP BY BUTTON FROM DEAD MODE
      if (STATE==DEAD && digitalRead(BUTTON_PIN)==HIGH){
          Serial.println("WAKE UP FROM DEAD MODE!");
          WiFiConnect();
          POLLING_INTERVAL = 1000; 
          STATE = IDLE;
          mills_active=0; 
          WiFi.noLowPowerMode();
      }

      //Periodically send an IMALIVE message
      if (STATE==IDLE && mills_from_last_imalive>5000){

          broadcast("imalive:");
          mills_from_last_imalive = 0;
      }
      else{
          mills_from_last_imalive+= POLLING_INTERVAL;
      }

      mills_active+=POLLING_INTERVAL;
      delay(POLLING_INTERVAL);
}

int receivePacket(char* text){
  int packetSize = Udp.parsePacket();
  if (packetSize) {
      // read the packet
      int len = Udp.read(text, 255);
      if (len > 0) text[len] = 0;
  }
  return packetSize;
}

void broadcast(char* text){

    //WiFi.noLowPowerMode(); 
    Udp.beginPacket(broadcastIp, REMOTE_PORT);
    Udp.write(text);
    Udp.endPacket();
    //WiFi.lowPowerMode(); 
    Serial.println(text);
}

void repeated_broadcast(char* text){

    for(int j=1;j<=3;j++){
       Udp.beginPacket(broadcastIp, REMOTE_PORT);
       Udp.write(text);
       Udp.endPacket();
       delay(j*20);
    }
}


void WiFiConnect(){
  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 2 seconds for connection:
    delay(2000);
  }
  Serial.println("Connected to wifi");
  printWiFiStatus();
  Udp.begin(LOCAL_PORT);
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
