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
// Name:         wemos01.ino
// Purpose:      A clock for speed climb
//
// Author:      Luzzi Valerio
//
// Created:     19/06/2021
//-------------------------------------------------------------------------------
#include "secrets.h" 
#include "functions.h" 

//-------------------------------------------------------------------------------
//
//
//    +-------+     +------+       +----------+       +---------+       +---------+       +---------+
//    |BEGIN  |---->| IDLE |------>| PREARMED |------>| ARMED   |------>| RUNNING |------>|   STOP  |
//    +-------+     +------+       +----------+       +---------+       +---------+       +---------+    
//                       ^              |                   |                 |                 |                          
//                       |              |                   |                 |                 |                         
//                       +--------------+<------------------+<----------------+<----------------+
//
//


bool isFootBoardPressed  = false;
bool isStopButtonPressed = false;

void setup() {

  changeStateTo(BEGIN);

  // initialize the button pin as a input:
  pinMode(START_BUTTON, INPUT_PULLUP);
  pinMode(STOP_BUTTON,  INPUT_PULLUP);
  
  //Initialize serial:
  Serial.begin(115200);

  WiFiSetup();
  
  changeStateTo(IDLE);

}

void loop() {

      LOOP_TIME = millis();
      isFootBoardPressed  = (digitalRead(START_BUTTON)==LOW);
      isStopButtonPressed = (digitalRead(STOP_BUTTON) ==LOW);

      // Check for a packet received one time each second ------------------
      if (mills_in_this_state%1000==0){
          int n = Udp.parsePacket();
          if (n){
              Udp.read(packet,PACKET_SIZE);
              String id = String(packet[0]);
              if (id.toInt() == ID ){
                   //We receive a packet
                   //may be a reset command
                   changeStateTo(IDLE);
              }
            
          }//udp receive
      }
      //-------------------------------------------------------------------

      //If footbar is pressed goto PREARMED
      if (STATE==IDLE && isFootBoardPressed){
          changeStateTo(PREARMED);
      }
      //After 2s in PREARMED we go ARMED
      else if (STATE==PREARMED && isFootBoardPressed && mills_in_this_state>2000){
          changeStateTo(ARMED);
      }
      //otherwhise return in IDLE
      else if (STATE==PREARMED && !isFootBoardPressed){
          changeStateTo(IDLE);
      }
      //if ARMED and we leave footbar goto RUNNING
      else if (STATE==ARMED && !isFootBoardPressed){
          ts = millis();
          changeStateTo(RUNNING);
      }
      //a pression on footboard while RUNNING of 2s cause return in IDLE
      else if (STATE==RUNNING && isFootBoardPressed && mills_in_this_state>2000){
          Serial.println("Returning to IDLE..");
          changeStateTo(IDLE);
      }
      //On STOP button goto STOP state
      else if (STATE==RUNNING && isStopButtonPressed){
          te = millis();
          Serial.println("!!!STOP STOP STOP!!!");
          changeStateTo(STOP);
      }
      else if (STATE==STOP && isFootBoardPressed){
          changeStateTo(IDLE);
      }
      //Every 500ms broadcast the state 
      else if (mills_in_this_state%500==0){
          broadcast_my_state();
      }

      delay(1);

      LOOP_TIME = millis()-LOOP_TIME;
      mills_in_this_state+=LOOP_TIME;
}//loop
