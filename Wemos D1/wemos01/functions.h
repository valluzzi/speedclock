//-------------------------------------------------------------------------------
// Licence:
// Copyright (c) 2019-2021 Luzzi Valerio 
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
// Name:         functions.h
// Purpose:      A clock for speed climb
//
// Author:      Luzzi Valerio
//
// Created:     19/06/2021
//-------------------------------------------------------------------------------
#include <ESP8266WiFi.h> //ESP8266
#include <WiFiUdp.h>

#define PACKET_SIZE 32

#define ID 1  //foot-board id 0,1,2,...
#define localPort 10999      // local port to listen on
#define remotePort 11000     // remote display port

#define DEBUG 1
#define START_BUTTON 2   //Wemos D4 pin
#define STOP_BUTTON 0    //Wemos D3 pin


unsigned long ts;  //timestamp of start
unsigned long te;  //timestamp of end

WiFiUDP Udp;
const IPAddress broadcastIp(255,255,255,255);

/**
 * printWiFiStatus
 */
void printWiFiStatus() {
  if (DEBUG){
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
}

bool WiFiSearchFor(char* ssid){

  for(int i=0;i<3;i++){
    
    delay(1000);
    
    if (DEBUG){
      Serial.println("** Scan Networks **");
    }
    
    int n = WiFi.scanNetworks();
  
    if (n == -1) {
        return false;
    }
  
    // print the list of networks seen:
    if (DEBUG){
        Serial.print("number of available networks:");
        Serial.println(n);
    }
    // print the network number and name for each network found:
    for (int j = 0; j < n; j++) {
        if (DEBUG){
            Serial.print(j);
            Serial.print(") ");
            Serial.print(WiFi.SSID(j));
            Serial.print("\tSignal: ");
            Serial.print(WiFi.RSSI(j));
            Serial.println(" dBm");
        }
        if (WiFi.SSID(j).equals(ssid)){
          return true;
        }
    }
    
  }
  return false;
}

/**
 * WiFISetup - connect to a WiFi Networka and prepare UDP port
 */
void WiFiSetup(){

  WiFi.mode(WIFI_STA);


  // check for the presence of the shield:
  while (WiFi.status() == WL_NO_SHIELD) {
      Serial.println("WiFi shield not present");
      delay(1000);
  }

  if (WiFiSearchFor(SECRET_SSID_1)){
      WiFi.begin(SECRET_SSID_1, SECRET_PASS_1);
  }else{
      WiFi.begin(SECRET_SSID_2, SECRET_PASS_2);
  }
  
  // attempt to connect to WiFi network:
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(WiFi.SSID());
    
    // wait 2 seconds for connection:
    Serial.println("------------------------------------------------");
    delay(2000);
  }
  
  Udp.begin(localPort);
  
  Serial.println("Connected to wifi");
  printWiFiStatus();

}




char packet[PACKET_SIZE]; //buffer to broadcast  
unsigned long mills_in_this_state=0;
unsigned long LOOP_TIME;

enum state {
  BEGIN,
  IDLE,
  PREARMED,
  ARMED,
  RUNNING,
  STOP
};

state STATE;

/**
 * broadcast - an udp packet 
 */
void broadcast(char* text){

    Udp.beginPacket(broadcastIp, remotePort);
    Udp.write(text);
    Udp.endPacket();
    if (DEBUG)
      Serial.println(text);
}

/**
 * broadcast_my_state
 */
void broadcast_my_state(){

  switch (STATE) {

      case BEGIN:
        sprintf(packet,"%d:BEGIN:%lu",ID,0);
        break;
      case IDLE:
        sprintf(packet,"%d:IDLE:%lu",ID,0);
        break;
      case PREARMED:
        sprintf(packet,"%d:PREARMED:%lu",ID,0);
        break;
      case ARMED:
        sprintf(packet,"%d:ARMED:%lu",ID,0);
        break;
      case RUNNING:
        sprintf(packet,"%d:RUNNING:%lu",ID,millis()-ts);
        break;
      case STOP:
        sprintf(packet,"%d:STOP:%lu",ID,te-ts);
        break;
      default:
        sprintf(packet,"%d:IDLE:%lu",ID,0);
        
    }//end switch
    
    broadcast(packet);
}//end broadcast my state

/**
 * changeStateTo - change state and broadcast it
 */
void changeStateTo(state s){
  
    STATE = s;
    mills_in_this_state = 0;
    broadcast_my_state();
}
