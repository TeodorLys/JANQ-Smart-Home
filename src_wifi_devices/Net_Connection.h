#pragma once
#include "Net_Parser.h"
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
// Web server
#else
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include "Status_codes.h"
// Web server
#endif


  char ssid[32] = "ESPJ";
  char pass[32] = "2997924582";
  IPAddress apIP(192, 168, 1,  1);
  IPAddress netMsk(255, 255, 255, 0);
  WiFiServer server(80);
class response_handler {
public:
  virtual void client_print(WiFiClient &client) {}
};

struct udp_packet{
  String _server_ip = "192.168.0.199";
  String _caller_type = "0";
  String _mac_address;
  String *_name;
  String _packet;
  udp_packet(){
    _mac_address = WiFi.macAddress();
  }
  
  void _internal_set_caller_type(String type){
    _caller_type = type;
  }

  void _internal_set_name(String *name){
    _name = name;
  }

  String server_ip() const {
    return _server_ip;
  }

  String get_constructed_packet(int status){
    String _n = *_name;
    if(_n == "")
      _n = "UNASSIGNED";
    
    _packet = _mac_address + ";" + WiFi.localIP().toString() + ";" + String(status) + ";" + _n + ";" + _caller_type + ";";
    return _packet;
  }
};

class net_connection {
private:
  String header;
  String current_line;
  String request_form;
  WiFiClient client;
  response_handler *_handler = nullptr;
  static udp_packet packet;
  const byte DNS_port = 53;
  DNSServer dns_server;
  const char *softAP_ssid = "JANQ_ap";
  const char *softAP_password = "299792458";

#ifdef ESP32
  Preferences preference;
#endif
  static WiFiUDP *udp;
  bool server_started = false;
public:

void connect(){
  WiFi.persistent(false);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  #ifdef ESP32
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);

  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
      Serial.print("WiFi connected. IP: ");
      Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
  },
      WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP);

  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
      Serial.print("WiFi lost connection. Reason: ");
      Serial.println(info.disconnected.reason);
      WiFi.persistent(false);
      WiFi.disconnect(true);
      WiFi.begin(ssid, pass);
      int index = 0;
      while (WiFi.status() != WL_CONNECTED && index < 100) {
        delay(250);
        Serial.print(".");
        index++;
      }
  },
      WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED);
#endif
  loadCredentials();
  /*
  If no credentials was found...
  */
  if(ssid == "NULL" || pass == "NULL"){
    WiFi.mode(WIFI_AP_STA);
    start_captive_portal();
  }
  WiFi.begin(ssid, pass);
  /*
  Try to connect 100 times, if it failes it will start the portal
  to reconfigure the connection.

  TODO: make a conditon, if a certain wifi isnt available wait 
  until it becomes available.
  */
  int index = 0;
  while (WiFi.status() != WL_CONNECTED && index < 100) {
    delay(250);
    Serial.print(".");
    index++;
  }
  if(WiFi.status() != WL_CONNECTED){
      WiFi.mode(WIFI_AP_STA);
      start_captive_portal();
  } else {
    Serial.println("Connected!");
    Serial.println(WiFi.localIP());
    standalone_start_server();
   }
}


public:
  net_connection(response_handler *handler, String *name) : _handler(handler) {
    udp = new WiFiUDP();  
    packet._internal_set_name(name);
  }

  void set_server_ip(String ip){
    packet._server_ip = ip;
  }

  void set_caller_type(String type){
    packet._internal_set_caller_type(type);
  }

  static udp_packet get_packet() {
    return packet;
  }

  void standalone_start_server(){
    send_status_to_server("starting server", STATUS_INITIAL_DEVICE);
    server.begin();
  }

  WiFiServer *temp_server_handle() const {return &server;}

  void set_request_form(String req){
    request_form = req;
  }

  bool new_client_connected(){
    client = server.available();
    if(client){
      header = "";
      current_line = "";
      while(client.connected()){
        if(client.available()){
          char c = client.read();
          header += c;
          if(c == '\n'){
            if(current_line.length() == 0){
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();
              client.println("<!DOCTYPE html><html>");
              String req_form = "<meta requestform=\"" + request_form + "\">";
              client.println(req_form);
              _handler->client_print(client);
              break;
            }else {
              current_line = "";
            }
          }else if(c != '\r'){
            current_line += c;
          }
        }
      }
      client.stop();
      /*
      If for some reason you want to change Wifi you can always send 
      this command to reset the EEPROM / Preferences save for credentials 
      */
      if(header.indexOf("resetaction.php?key=299792458") >= 0){
        resetCredentials();
        #ifdef ESP8266
          ESP.reset();
        #else
          ESP.restart();
        #endif
      }
        
      return true;
    }else {
      return false;
    }
  }

  String get_header(){
    return header;
  }

  void send_status_to_server(String message, int status = STATUS_SUCCESS){
    udp->beginPacket(packet.server_ip().c_str(), 4123);
    
    //MACADDRESS;IPADDRESS;STATUS_CODE;NAME_OF_DEVICE;SLAVE_OR_HOST;MESSAGE
    //00:00:00:00:00:00;192.168.0.0;200;NAME;0;this is a funny message.
    String buffer = packet.get_constructed_packet(status);
    buffer += message;
    #ifdef ESP8266
    udp->write(buffer.c_str(), buffer.length());
    #else
    udp->print(buffer);
    #endif
    udp->endPacket();
    udp->flush();
  }

  static String get_name() {
    return *packet._name;
  }

  static WiFiUDP* get_upd_handler() {
    return udp;
  }

protected:
#ifdef ESP8266

/*
TODO: Make a save file class, so you can do device specific saves, 
and the size gets added to the credentials.
*/

/** Load WLAN credentials from EEPROM */
void loadCredentials() {
  /*
  +20 here because I use those 20 for the name of device.
  */
  EEPROM.begin(512);
  EEPROM.get(20, ssid);
  EEPROM.get(20 + sizeof(ssid), pass);
  char ok[2 + 1];
  EEPROM.get(20 + sizeof(ssid) + sizeof(pass), ok);
  EEPROM.end();
  if (String(ok) != String("OK")) {
    ssid[0] = 0;
    pass[0] = 0;
  }
  Serial.println("Recovered credentials:");
  Serial.println(ssid);
  Serial.println(strlen(pass) > 0 ? "********" : "<no password>");
}

void resetCredentials() {
  String _ssid = "NULL";
  String _pass = "NULL";
  _ssid.toCharArray(ssid, 32);
  _pass.toCharArray(pass, 32);
  EEPROM.begin(512);
  EEPROM.put(20, ssid);
  EEPROM.put(20 + sizeof(ssid), pass);
  char ok[2 + 1] = "OK";
  EEPROM.put(20 + sizeof(ssid) + sizeof(pass), ok);
  EEPROM.commit();
  EEPROM.end();
}

/** Store WLAN credentials to EEPROM */
void saveCredentials() {
  EEPROM.begin(512);
  EEPROM.put(20, ssid);
  EEPROM.put(20 + sizeof(ssid), pass);
  char ok[2 + 1] = "OK";
  EEPROM.put(20 + sizeof(ssid) + sizeof(pass), ok);
  EEPROM.commit();
  EEPROM.end();
}
#else
/*
 Load WLAN credentials from Preferences 
*/
void loadCredentials() {
  /*
  You have to begin and end the preference each time you want to interact
  with it. I dont know why, you should just be able to call begin in constructor 
  but it did not work for me...
  */
  preference.begin("credentials", false);
  preference.getString("ssid", ssid, 32);
  preference.getString("password", pass, 32);
  preference.end();
  Serial.println("Recovered credentials:");
  Serial.println(ssid);
  Serial.println(pass);
}

/* 
Clear the ssid and password
*/
void resetCredentials() {
  preference.begin("credentials", false);
  preference.putString("ssid", "NULL");
  preference.putString("password", "NULL");
  preference.end();
  Serial.println("Saved credentials:");
  Serial.println(ssid);
  Serial.println(pass);
}

/* 
Store WLAN credentials to Preference 
*/
void saveCredentials() {
  String _ssid = ssid;
  String _pass = pass;
  preference.begin("credentials", false);
  preference.putString("ssid", _ssid);
  preference.putString("password", _pass);
  preference.end();
  Serial.println("Saved credentials:");
  Serial.println(ssid);
  Serial.println(pass);
}
#endif
private:

void start_captive_portal(){
  Serial.println("Portal Started");
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid, softAP_password);
  delay(500); // Without delay I've seen the IP address blank
  dns_server.setErrorReplyCode(DNSReplyCode::NoError);
  dns_server.start(DNS_port, "*", apIP);
  server.begin();
  String _ssid;
  String _pass;
  net_parser parser;
  parser.add_parameter("ssid");
  parser.add_parameter("pass");
  while(_ssid == "" && _pass == "" || _ssid != "exit"){
    if(new_client_connected()){
      Serial.println("new client found!");
      parser.parse_header(get_header());
      if(parser.get_parameter("ssid") != -1){
        Serial.println("Found ssid");
        _ssid = parser.get_string_parameter("ssid");
        Serial.println(_ssid);
      }
      if(parser.get_parameter("pass") != -1){
        Serial.println("Found pass");
        _pass = parser.get_string_parameter("pass");
        Serial.println(_pass);
      }
      if(parser.get_parameter("pass") != -1 && parser.get_parameter("ssid") != -1){
        Serial.println("Found all");
        break;
      }
    }
  }

  if(_ssid != "" && _pass != ""){
    _ssid.toCharArray(ssid, 32);
    _pass.toCharArray(pass, 32);
    saveCredentials();
    Serial.print(ssid);
    Serial.print(", ");
    Serial.print(pass);
    Serial.println(" Saved parameters");
  #ifdef ESP8266
    ESP.reset();
  #else
    preference.end();
    ESP.restart();
  #endif
  delay(1000);
  }
}

};

WiFiUDP *net_connection::udp = nullptr;
udp_packet net_connection::packet;

void send_status_to_server(String message, int status = STATUS_SUCCESS){
  net_connection::get_upd_handler()->beginPacket(net_connection::get_packet().server_ip().c_str(), 4123);
  String buffer = net_connection::get_packet().get_constructed_packet(status);
  buffer += message;
  #ifdef ESP8266
  net_connection::get_upd_handler()->write(buffer.c_str(), buffer.length());
  #else
  net_connection::get_upd_handler()->print(buffer);
  #endif
  net_connection::get_upd_handler()->endPacket();
  net_connection::get_upd_handler()->flush();
}
