#include <Arduino.h>
//NET_API DEPS
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
//NET_API DEPS
#include <net_api.h>
#include <Status_codes.h>

byte last_i_speed = 0;

net_api api;


void setup() {
  pinMode(3, OUTPUT);
  api.initialize(2, "Servo", "function");
}

void loop() {
  if(api.got_request()) {
    int i_speed = api.parameter("Servo", GET_INT);
    
      if(i_speed == 1)
        analogWrite(3, 1023);
      else if(i_speed == 0)
        analogWrite(3, 0);
      else {
        api.push_message_to_server("execution out of bounds!", STATUS_FAILED);
        return;
       }
      String status_repo = "execute function -> " + String(i_speed == 1 ? "ON" : "OFF");
      api.push_message_to_server(status_repo, STATUS_SUCCESS);
      last_i_speed = i_speed;
  }
}
