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
// Name:         wemos00.ino
// Purpose:      1step - read buttons
//
// Author:      Luzzi Valerio
//
// Created:     19/06/2021
//-------------------------------------------------------------------------------
//#include <ESP8266WiFi.h> //ESP8266
//#include <WiFiUdp.h>


const int PIN_D3 = 0;
const int PIN_D4 = 2;
const int PIN_D7 = 13;



void setup() {


  //Initialize serial and wait for port to open:
  Serial.begin(115200);

  // initialize the button pin as a input:
  pinMode(PIN_D7, INPUT_PULLUP);
  //pinMode(PIN_D3, INPUT_PULLUP);
  //pinMode(PIN_D4, INPUT_PULLUP);

  
  
  
}

void loop() {

    if (digitalRead(PIN_D7)==LOW){
       Serial.println("LOW");  
    }else{
       //Serial.println("HIGH");  
    }
    delay(10);
      
}//loop
