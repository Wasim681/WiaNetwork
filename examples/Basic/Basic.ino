
#define WIA_ENABLE_OTA false
#define WIA_ENABLE_NET_AYSNC false

#include <WiaWiFi.h>

WiaWiFi wianet;

void setup() {
  Serial.begin(115200);
  // initalize the network and OTA if check is set true
  wianet.init();
}

void loop() {
  // handle the states of wifi and OTA 
  wianet.loop();
}
