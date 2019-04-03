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
// Name:         SpeedClock.ino
// Purpose:      A clock for speed climb
//
// Author:      Luzzi Valerio
//
// Created:     20/03/2019
//-------------------------------------------------------------------------------

#include <SPI.h>
//#include <WiFi101.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include "HX711.h"
#include "arduino_secrets.h" 

// HX711 -----------------------
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;
const int BUTTON_PIN = 4;   
 
HX711 scale;
long tare; 
long weight=0;
long THRESHOLD_WEIGHT = 20;  //20kg
long ALPHA = 9300;  //conversion factor for scale into kg.
long success =0;
long failed = 0;
//------------------------------



int POLLING_INTERVAL = 1000;
unsigned long t0;
unsigned long t1;
unsigned long seconds_in_idle = 0;
unsigned long seconds_in_prearmed =0;
unsigned long RUNNING_COUNTER=0;

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
unsigned int remotePort = 11000;      // remote display port
char cmd[64]; //buffer command to broadcast  

WiFiUDP Udp;
IPAddress broadcastIp(255,255,255,255);

void setup() {

  // initialize the button pin as a input:
  pinMode(BUTTON_PIN, INPUT);
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}

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

  //---------- WEIGHT SCALE -----------------
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  POLLING_INTERVAL=1000;
  seconds_in_prearmed = 0;
  Serial.print("tara= ");
  tare = taring();
  Serial.println(tare);
  broadcast("ready:");
  STATE=IDLE;
}

void loop() {

      if (STATE != RUNNING || RUNNING_COUNTER%POLLING_INTERVAL==0){
          while ( !scale.is_ready() ) {
              delay(1);
          }
          weight = (-scale.read()+tare) / ALPHA;
          RUNNING_COUNTER=0;
      }
     
      if (STATE == IDLE && weight>THRESHOLD_WEIGHT && seconds_in_prearmed>=2){
          STATE = ARMED;//armed  
          POLLING_INTERVAL = 50;
          seconds_in_idle =0;
          seconds_in_prearmed = 0;
          Serial.println("CLOCK ARMED!");;
          broadcast("armed:");
      }
      
      if (STATE == ARMED && weight<5){
          //Start!!!
          t0 = millis();
          STATE = RUNNING;//running  
          POLLING_INTERVAL =10;
          Serial.println("CLOCK IS RUNNING!");
          broadcast("start:");
      }

      if (STATE==RUNNING){
          RUNNING_COUNTER++;
      }
      
      if (STATE==RUNNING && weight>THRESHOLD_WEIGHT){
          t1 = millis();
          STATE = IDLE;//idle 
          POLLING_INTERVAL =1000;
          Serial.println("CLOCK RESET!");
          Serial.print("T=");
          Serial.print((t1-t0));
          Serial.println("ms.");
          Serial.println("---------------------------------------");
        
          sprintf(cmd,"end:%d",(t1-t0));
          broadcast(cmd);
      }
      
      if (STATE==RUNNING && false &&  digitalRead(BUTTON_PIN)==HIGH){
          t1 = millis();
          STATE==IDLE;
          POLLING_INTERVAL =1000;
          Serial.println("CLOCK STOPPED BY BUTTON!");
          Serial.print("T=");
          Serial.print((t1-t0));
          Serial.println("ms.");
          Serial.println("---------------------------------------");
          sprintf(cmd,"end:%d",(t1-t0));
          broadcast(cmd);   
      }
      
      if (STATE==IDLE){
          POLLING_INTERVAL=1000;
          //Serial.print(weight);
          //Serial.println(" kg." );
          sprintf(cmd,"idle:%d",weight);
          broadcast(cmd);
          seconds_in_idle++;
          seconds_in_prearmed = (weight>THRESHOLD_WEIGHT)?seconds_in_prearmed+1:0;         
      }
    
      //Periodically broadcast a i_m_alive message
      if (STATE==IDLE && seconds_in_idle%10==0){
          seconds_in_idle = 0;
          broadcast("ready:");
      }

      delay(POLLING_INTERVAL);
  
}

long taring(){
  long times = 50;
  long mean = 0;
  for(int j=0;j < times; j++){
    while ( !scale.is_ready() )delay(1);
    mean += scale.read();
    delay(100);
  } 
  return mean/times;
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
