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
#include <ESP_EEPROM.h>

#define PACKET_SIZE 32

#define localPort 10999      // local port to listen on
#define remotePort 11000     // remote display port

#define DEBUG 1
#define START_BUTTON 2   //Wemos D4 pin
#define STOP_BUTTON 0    //Wemos D3 pin


int ID=1;  // Footboard ID 0,1,2,...
unsigned long ts;  //timestamp of start
unsigned long te;  //timestamp of end



WiFiUDP Udp;
IPAddress broadcastIp;
//const IPAddress broadcastIp(192,168,4,255);// WiFi SoftAp
//const IPAddress broadcastIp(255,255,255,255);  //WiFi STA
const IPAddress local_IP(192,168,4,1);
const IPAddress gateway(192,168,4,1);
const IPAddress subnet(255,255,255,0);

struct record{
  int id;
  char ssid[16];  
  char password[32];
} record1;


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


void WiFiSoftAP(){

     ID=1;
     broadcastIp=IPAddress(192,168,4,255);
     delay(5000);
      
     Serial.println("======================================");
     Serial.print("Setting soft-AP configuration ... ");
     Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");
     WiFi.softAP(SSID_NAME);
     
     Serial.print("Soft-AP IP address = ");
     Serial.println(WiFi.softAPIP());
     Serial.println("======================================");
     Udp.begin(localPort);
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

    //EEPROM.begin(sizeof(record));
    ID=1;
    WiFi.mode(WIFI_STA);
    if (strcmp("SpeedClimb",SSID_NAME)==0){
      
      broadcastIp=IPAddress(192,168,4,255);
      
      WiFi.begin(SSID_NAME);  
    }
    else{
      broadcastIp=IPAddress(255,255,255,255);
      WiFi.begin(SSID_NAME,PASSWORD);  
    }

    // check for the presence of the shield:
    while (WiFi.status() == WL_NO_SHIELD) {
        Serial.println("WiFi shield not present");
        delay(1000);
    }
    
    // attempt to connect to WiFi network:
    while ( WiFi.status() != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(SSID_NAME);
      // wait 2 seconds for connection:
      Serial.println("------------------------------------------------");
      delay(2000);
    }
    
    // SAVE SSID & PASSWORD TO EEPROM
    //record1.id = ID;
    //strcpy(record1.ssid,SECRET_SSID_1);
    //strcpy(record1.password,SECRET_PASS_1);
    //Serial.print("Saving data...");
    //EEPROM.put(0,record1);
    //EEPROM.commit();
    //Serial.println("done!");
    
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
