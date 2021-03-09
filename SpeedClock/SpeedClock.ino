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
//#include <WiFi101.h> //MKR1000
#include <WiFiNINA.h> //MKR1010
#include <WiFiUdp.h>
#include "HX711.h"
#include "arduino_secrets.h" 

// HX711 -----------------------
const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN  = 3;
const int BUTTON_PIN = 2;   


HX711 scale;
long tare; 
long weight=0;
long THRESHOLD_WEIGHT = 10;  //20kg
long ALPHA = 9300;  //conversion factor for scale into kg.
const int DEBUG = 1;
//------------------------------
unsigned long t0;
unsigned long t1;
unsigned long LOOP_TIME;
unsigned long RUNNING_COUNTER;
unsigned long mills_in_this_state =0;


enum state {
  BEGIN,
  CALIBRATING,
  IDLE,
  PREARMED,
  ARMED,
  RUNNING,
};

state STATE;

//--------------------------------------------------------------------------------------------
//
//    +-------+     +-------------+       +------+       +----------+       +---------+       +---------+
//    |BEGIN  |---->|CALIBRATING  |------>| IDLE |------>| PREARMED |------>| ARMED   |------>| RUNNING |
//    +-------+     +-------------+       +------+       +----------+       +---------+       +---------+     
//                                           ^              |                 |                 |          
//                                           |              |                 |                 |         
//                                           +--------------+-----------------+-----------------+
//
//------------ WiFI --------------------------------------------------------------------------
int status = WL_IDLE_STATUS;
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;        // your network password (use for WPA, or use as key for WEP)
unsigned int localPort  = 10999;      // local port to listen on
unsigned int remotePort = 11000;      // remote display port
char cmd[32]; //buffer command to broadcast  

WiFiUDP Udp;
//WiFiServer wifiServer(80);
//IPAddress broadcastIp(192,168,1,255);
IPAddress broadcastIp(255,255,255,255);

void setup() {

  changeStateTo(BEGIN);

  // initialize the button pin as a input:
  pinMode(BUTTON_PIN, INPUT);
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}

  // check for the presence of the shield:
  while (WiFi.status() == WL_NO_SHIELD) {
      println("WiFi shield not present");
      delay(1000);
  }

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    print("Attempting to connect to SSID: ");
    println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(2000);
  }
  println("Connected to wifi");
  printWiFiStatus();

  //Setup a TCP SERVER
  //wifiServer.begin();
  

  println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  Udp.begin(localPort);

  //---------- WEIGHT SCALE -----------------
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
 
  changeStateTo(CALIBRATING);
  print("tara= ");
  tare = taring();
  println(tare);
  changeStateTo(IDLE);

}

void loop() {

      //weighting ~= 85ms
      LOOP_TIME = millis();
      if (STATE!=RUNNING || (STATE==RUNNING && RUNNING_COUNTER%20==0)){
          while ( !scale.is_ready() ){
              yield();  
          }
          weight = (scale.read()-tare) / ALPHA;
          RUNNING_COUNTER=0;
      }
      RUNNING_COUNTER++;

      //Periodically broadcast a i_m_alive message
      if (STATE==IDLE && mills_in_this_state>=2500){
          changeStateTo(IDLE);
      }
      
      //PRE-ARMED!!! 
      if (STATE == IDLE && weight>=THRESHOLD_WEIGHT){
          changeStateTo(PREARMED);
      }

      //ARMED
      if (STATE == PREARMED && mills_in_this_state>=500){
          changeStateTo(ARMED);
      }

      //RETURN TO IDLE
      if (STATE == PREARMED && weight<THRESHOLD_WEIGHT){
          changeStateTo(IDLE);
      }

      //START ---> RUNNING
      if (STATE == ARMED && weight<(THRESHOLD_WEIGHT/2)){
          t0 = millis();
          changeStateTo(RUNNING);
      }

      //Periodically broadcast a i_m_alive message
      if (STATE==ARMED && mills_in_this_state>1000){
          changeStateTo(ARMED);
      } 

      //BREAK RUNNING STATE BY GETTING ON THE PLATFORM 
      if (STATE==RUNNING && weight>=(THRESHOLD_WEIGHT/2)){
          STATE = IDLE;
          mills_in_this_state=0;
          sprintf(cmd,"%s:stop:0",ID);
          repeated_broadcast(cmd,2);
      }

      //STOP RUNNING BY PRESSING BUTTON 
      if (STATE==RUNNING && digitalRead(BUTTON_PIN)){
          t1 = millis();
          STATE = IDLE;
          mills_in_this_state=0;
          sprintf(cmd,"%s:stop:%d",ID,(t1-t0));
          repeated_broadcast(cmd,10);
      }

      

      LOOP_TIME = millis()-LOOP_TIME;
      mills_in_this_state+=LOOP_TIME;
}//loop

long taring(){
  
  long times = 50;
  long mean = 0;
  for(int j=0;j < times; j++){
    while ( !scale.is_ready() ){
        yield();  
    }
    mean += scale.read();
    delay(100);
  } 
  return mean/times;
}

void broadcast(char* text){

    Udp.beginPacket(broadcastIp, remotePort);
    Udp.write(text);
    Udp.endPacket();
    //println(text);
}

void repeated_broadcast(char* text, int n){

    for(int j=1;j<=n;j++){
      Udp.beginPacket(broadcastIp, remotePort);
      Udp.write(text);
      Udp.endPacket();
      if (j>1){
        delay(j*random(5,30));
      }
    }
}

void changeStateTo(state s){
  
    STATE = s;
    mills_in_this_state = 0;
    switch (s) {

      case BEGIN:
        sprintf(cmd,"%s:begin:0",ID);
        break;
      case CALIBRATING:
        sprintf(cmd,"%s:calibrating:0",ID);
        break;
      case IDLE:
        sprintf(cmd,"%s:ready:0",ID);
        break;
      case PREARMED:
        sprintf(cmd,"%s:prearmed:0",ID);
        break;
      case ARMED:
        sprintf(cmd,"%s:armed:0",ID);
        break;
      case RUNNING:
        sprintf(cmd,"%s:start:0",ID);
        break;
      default:
        sprintf(cmd,"%s:ready:0",ID);
        
    }
    
    broadcast(cmd);
}

void print(const char* text){
  if (DEBUG)Serial.print(text);
}

void print(int text){
  if (DEBUG)Serial.print(text);
}

void print(long text){
  if (DEBUG)Serial.print(text);
}

void print(float text){
  if (DEBUG)Serial.print(text);
}

void println(const char* text){
  if (DEBUG)Serial.println(text);  
}

void println(int text){
  if (DEBUG)Serial.println(text);  
}

void println(long text){
  if (DEBUG)Serial.println(text);  
}

void println(float text){
  if (DEBUG)Serial.println(text);  
}


void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  print("SSID: ");
  println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  print("signal strength (RSSI):");
  print(rssi);
  println(" dBm");
}
