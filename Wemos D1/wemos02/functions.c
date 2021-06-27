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
#include "functions.h"

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

/**
 * WiFISetup - connect to a WiFi Networka and prepare UDP port
 */
void WiFiSetup(){

  WiFi.mode(WIFI_STA);
  WiFi.begin(SECRET_SSID_1, SECRET_PASS_1);

  // check for the presence of the shield:
  while (WiFi.status() == WL_NO_SHIELD) {
      Serial.println("WiFi shield not present");
      delay(1000);
  }

  // attempt to connect to WiFi network:
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID_1);
    // wait 2 seconds for connection:
    Serial.println("------------------------------------------------");
    delay(2000);
  }
  
  Udp.begin(localPort);
  
  Serial.println("Connected to wifi");
  printWiFiStatus();

}

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
