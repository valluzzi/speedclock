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

#define ID 0  //foot-board id 0,1,2,...
#define localPort 10999      // local port to listen on
#define remotePort 11000     // remote display port

#define DEBUG 1
#define START_BUTTON 2   //Wemos D4 pin
#define STOP_BUTTON 0    //Wemos D3 pin


unsigned long ts;  //timestamp of start
unsigned long te;  //timestamp of end

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

WiFiUDP Udp;
const IPAddress broadcastIp(255,255,255,255);


void printWiFiStatus() ;
void WiFiSetup();
void broadcast(char* text);
void broadcast_my_state();
void changeStateTo(state s);
