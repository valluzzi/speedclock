# speedclock
Clock for speed climbing race https://en.wikipedia.org/wiki/Speed_climbing

This project was born to create a cheap and DIY footbooard for speed climbing but It can be easily modified for other similar or unknown scopes. It is composed of hardware and software. The hardware ( the footboard ) is build over an ESP8266.
We choose Wemos D1 mini v1 but in the future other devices like ESP32 will be covered.
https://www.aliexpress.com/item/32649468489.html?spm=a2g0o.productlist.0.0.37ce1669KXjsjo&aem_p4p_detail=202107060356142767729499562560047923585

The footboard is a DIY footboard. We are making some prototypes and instructions to build will be included soon.
Anyway the footboard is build over the Wemos + 2 buttons. The first one acts as a trigger when the athlete starts and the second one is a stop button that stops the time when the athlete stops the race. The start button is linked to the wemos in D4 and GND. The stop button D3 and GND. No resistance is needed between. All buttons are NO buttons (Normally Open). The display is replaced by an Android app that can be loaded on a tablet or just a smartphone. Actually the app do nothing else more than displaying a clock, because this clock is totally actioned by the wemos. The wemos take the start time and the stop time and it continuosly fires UDP packets to the app to synchronize the display. 
