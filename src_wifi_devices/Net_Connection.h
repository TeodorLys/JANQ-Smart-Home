#pragma once
#include "Net_Parser.h"
#include "Status_codes.h"
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#else
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#endif


char ssid[32] = "ESPJ";
char pass[32] = "2997924582";
char _ip[16] = "192.168.0.300";
String broadcast_ip;
IPAddress apIP(192, 168, 1,  1);
IPAddress netMsk(255, 255, 255, 0);
WiFiServer server(80);

class response_handler {
public:
  virtual void client_print(WiFiClient &client) {}
};

struct udp_packet{
  //String _server_ip = "192.168.0.199";
  String _server_ip = "255.255.255.255";
  String _caller_type = "0";
  String _mac_address;
  String _name;
  String _packet;
  udp_packet(){
    _mac_address = WiFi.macAddress();
  }
  
  void _internal_set_caller_type(String type){
    _caller_type = type;
  }

  void _internal_set_name(String name){
    _name = name;
  }

  String server_ip() const {
    return _server_ip;
  }

  String get_constructed_packet(int status){
    String _n = _name;
    if(_n == "")
      _n = "UNASSIGNED";
    
    _packet = _mac_address + ";" + WiFi.localIP().toString() + ";" + String(status) + ";" + _n + ";" + _caller_type + ";";
    return _packet;
  }
};

class net_connection {
private:


  class response : public response_handler {
  private:
    String m;
    String status[20];
  public:

    void push_back(String s){
      if(!status[19].isEmpty()){
        for(int a = 1; a < 19; a++){
          status[a] = status[a - 1];
        }
        status[19] = s;
      }else {
        for(int a = 0; a < 20; a++){
          if(status[a].isEmpty()){
            status[a] = s;
            return;
          }
        }
      }

    }

    void set_message(String _m){
      m = _m;
    }

    void client_print(WiFiClient &client) {
      client.println("<style>");
      client.println("input[type=text], select {");
      client.println("  width: 100%;");
      client.println("  padding: 12px 20px;");
      client.println("  margin: 8px 0;");
      client.println("  border: 1px solid #ccc;");
      client.println("  border-radius: 4px;");
      client.println("  box-sizing: border-box;");
      client.println("  font-family: \"Arial\"\n}");
      client.println("input[type=submit] {");
      client.println("  width: 100%;");
      client.println("  background-color: #4CAF50;");
      client.println("  color: white;");
      client.println("  padding: 14px 20px;");
      client.println("  margin: 8px 0;");
      client.println("  border: none;");
      client.println("  font-family: \"Arial\"\n}");
      client.println("div {");
      client.println("  border-radius: 5px;");
      client.println("  background-color: #f2f2f2;");
      client.println("  padding: 20px;\n}");
      client.println("</style>");
      client.println("<body>");
      client.println("<font face=\"Arial\">");
      String err = String("<h3>") + m + String("</h3>");
      client.println(err);
      client.println("<div>");
      client.println("  <form action=\"/actionpage.php\">");
      client.println("    <label for=\"fname\">SSID</label>");
      client.println("    <input type=\"text\" id=\"fname\" name=\"ssid\" placeholder=\"SSID\">");
      client.println("    <label for=\"lname\">PASSWORD</label>");
      client.println("    <input type=\"text\" id=\"lname\" name=\"pass\" placeholder=\"Password\">");
      client.println("  	<label for=\"lname\">NAME</label>");
      client.println("    <input type=\"text\" id=\"nname\" name=\"name\" placeholder=\"DEVICE\">");
      client.println("    <input type=\"submit\" value=\"Submit\">");
      client.println("  </form>");
      for(int a = 0; a < 20; a++){
        if(status[a].isEmpty())
          break;
        client.println("<h3>" + status[a] + "</h3>");
      }
      client.println("</div>");
      client.println("</body>");
      client.println("</html>");
    }
  };


  String header;
  String current_line;
  String request_form;
  WiFiClient client;
  response_handler *_handler = nullptr;
  response res;
  static udp_packet packet;
  const byte DNS_port = 53;
  DNSServer dns_server;
  MDNSResponder mdns;
  bool mdns_success = false;
  bool _disable_static = false;
  bool _enable_mdns = false;
  const char *softAP_ssid = "JANQ_ap";
  const char *softAP_password = "299792458";

#ifdef ESP32
  Preferences preference;
#endif
  static WiFiUDP *udp;
  bool server_started = false;
public:

void connect(String __ssid = "", String __pass = "", void(*func)() = nullptr){
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
  if(__ssid == "" && __pass == ""){
    loadCredentials();
    /*
    If no credentials was found...
    */
    if(String(ssid).indexOf("NULL") >= 0 || String(pass).indexOf("NULL") >= 0){
      res.set_message("No SSID / PASSWORD was found from EEPROM");
      WiFi.mode(WIFI_AP_STA);
      start_captive_portal();
    }

    Serial.print(ssid);
    Serial.print(", ");
    Serial.println(pass);

    if(String(_ip).indexOf("192.168.0.300") < 0 && !_disable_static){
      IPAddress addr;
      IPAddress gate(192,168,0,1);
      addr.fromString(_ip);
      WiFi.config(addr, gate, netMsk);
    }

    WiFi.begin(ssid, pass);
  } else {
    loadCredentials();
    if(String(_ip).indexOf("192.168.0.300") < 0 && !_disable_static){
      IPAddress addr;
      IPAddress gate(192,168,0,1);
      addr.fromString(_ip);
      WiFi.config(addr, gate, netMsk);
    }
    WiFi.begin(__ssid.c_str(), __pass.c_str());
  }
  
  
  /*
  Try to connect 50 times, if it failes it will start the portal
  to reconfigure the connection.

  TODO: make a conditon, if a certain wifi isnt available wait 
  until it becomes available.
  */
  int index = 0;
  while (WiFi.status() != WL_CONNECTED && index < 50) {
    delay(250);
    Serial.print(".");
    if(func != nullptr){
      func();
    }
    index++;
  }
  if(WiFi.status() != WL_CONNECTED){
      WiFi.mode(WIFI_AP);
      res.set_message("Could not connect to loaded SSID: " + String(ssid));
      start_captive_portal();
  } else {
    Serial.println("Connected!");
    Serial.println(WiFi.localIP());
    WiFi.localIP().toString().toCharArray(_ip, 16);
    if(_enable_mdns){
      String _n = get_name();
      if(_n == "")
        _n = "UNASSIGNED";
      if(mdns.begin(_n))
        mdns_success = true;
    }

    saveCredentials();
    standalone_start_server();
    if(mdns_success)
      if(!MDNS.addService("http", "tcp", 80)) mdns_success = false;
    }
    delay(50);
    if(mdns_success)
      send_status_to_server("Starting MDNS: " + get_name() + ".local");
    else 
      send_status_to_server("Failed to start MDNS: " + get_name() + ".local", STATUS_FAILED);
}


public:
  net_connection(response_handler *handler) : _handler(handler) {
    udp = new WiFiUDP();  
  }

  void set_server_ip(String ip){
    packet._server_ip = ip;
  }

  void set_caller_type(String type){
    packet._internal_set_caller_type(type);
  }

  void set_normal_name(String n){
    send_status_to_server("Changed name from: " + packet._name + " to: " + n);
    packet._internal_set_name(n);
  }

  static udp_packet get_packet() {
    return packet;
  }

  void disable_static(){
    _disable_static = true;
  }

  void enable_mdns(){
    _enable_mdns = true;
  }

  void set_request_form(String req){
    request_form = req;
  }

  WiFiServer *temp_server_handle() const {return &server;}

  void standalone_start_server(){
    String s = "Starting Server with static: " + String(_ip);
    send_status_to_server(s, STATUS_SUCCESS);
    server.begin();
  }

  bool new_client_connected(response_handler* special = nullptr){
    client = server.available();
    if(!mdns.update())
      send_status_to_server("MDNS Update failed!", STATUS_FAILED);
    if(client){
      header = "";
      current_line = "";
      while(client.connected()){
        if(client.available()){
          char c = client.read();
          header += c;
          if(c == '\n'){
            if(current_line.length() == 0) {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();
              client.println("<!DOCTYPE html><html>");
              String req_form = "<meta requestform=\"" + request_form + "\">";
              client.println(req_form);
              /*
              Special are for the internal start_captive_portal function, to display the ssid, pass and name input fields 
              */
              if(special == nullptr)
                _handler->client_print(client);
              else 
                special->client_print(client);
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
        send_status_to_server("Resetting chip EEPROM, AP will start shortly"); 
        delay(1000);
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
    return packet._name;
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
  void loadCredentials() {
    char name[16];
    EEPROM.begin(512);
    EEPROM.get(0, ssid);
    EEPROM.get(sizeof(ssid), pass);
    char ok[3];
    EEPROM.get(sizeof(ssid) + sizeof(pass), _ip);
    EEPROM.get(sizeof(ssid) + sizeof(pass) + sizeof(_ip), name);
    EEPROM.get(sizeof(ssid) + sizeof(pass) + sizeof(_ip) + sizeof(name), ok);
    EEPROM.end();
    packet._name = name;
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
    char __ip[16] = "192.168.0.300";
    packet._name = "NULL";
    EEPROM.begin(512);
    EEPROM.put(0, ssid);
    EEPROM.put(sizeof(ssid), pass);
    char ok[2 + 1] = "OK";
    EEPROM.put(sizeof(ssid) + sizeof(pass), __ip);
    EEPROM.put(sizeof(ssid) + sizeof(pass) + sizeof(__ip), packet._name);
    EEPROM.put(sizeof(ssid) + sizeof(pass) + sizeof(__ip) + packet._name.length(), ok);
    EEPROM.commit();
    EEPROM.end();
  }
public:
/** Store WLAN credentials to EEPROM */
  void saveCredentials() {
    char name[16];
    packet._name.toCharArray(name, 16);
    EEPROM.begin(512);
    EEPROM.put(0, ssid);
    EEPROM.put(sizeof(ssid), pass);
    char ok[2 + 1] = "OK";
    EEPROM.put(sizeof(ssid) + sizeof(pass), _ip);
    EEPROM.put(sizeof(ssid) + sizeof(pass) + sizeof(_ip), name);
    EEPROM.put(sizeof(ssid) + sizeof(pass) + sizeof(_ip) + sizeof(name), ok);
    EEPROM.commit();
    EEPROM.end();
  }
private:
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
    char name[16];
    preference.begin("credentials", false);
    preference.getString("ssid", ssid, 32);
    preference.getString("password", pass, 32);
    preference.getString("ip", _ip, 16);
    preference.getString("name", name, 16);
    preference.end();
    packet._name = name;
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
    preference.putString("ip", "192.168.0.100");
    preference.putString("name", "UNASSIGNED");
    preference.end();
    Serial.println("Saved credentials:");
    Serial.println(ssid);
    Serial.println(pass);
  }

/* 
Store WLAN credentials to Preference 
*/
public:
  void saveCredentials() {
    String _ssid = ssid;
    String _pass = pass;
    send_status_to_server("Saving new name: " + packet._name);
    preference.begin("credentials", false);
    preference.putString("ssid", _ssid);
    preference.putString("password", _pass);
    preference.putString("ip", _ip);
    preference.putString("name", packet._name);
    preference.end();
    Serial.println("Saved credentials:");
    Serial.println(ssid);
    Serial.println(pass);
  }
#endif
private:

  void start_captive_portal() {
    WiFi.softAPConfig(apIP, apIP, netMsk);
    WiFi.softAP(softAP_ssid, softAP_password);
    delay(500); // Without delay I've seen the IP address blank
    //dns_server.setErrorReplyCode(DNSReplyCode::NoError);
    //dns_server.start(DNS_port, "*", apIP);
    server.begin();
    String _ssid;
    String _pass;
    String _name;
    net_parser parser;
    parser.add_parameter("ssid");
    parser.add_parameter("pass");
    parser.add_parameter("name");
    long scan_timer = millis();
    while((_ssid == "" && _pass == "") || _ssid != "exit"){
      //dns_server.processNextRequest();

      if(String(ssid).isEmpty()){
        if(millis() - scan_timer > 30000){
          int i = WiFi.scanNetworks();
          for(int a = 0; a < i; a++){
            if(WiFi.SSID(a) == ssid){
                #ifdef ESP8266
                ESP.reset();
              #else
                ESP.restart();
              #endif
            }
          }
        }
      }

      if(new_client_connected(&res)){
        Serial.println("new client found!");
        _return r = parser.parse_header(get_header());
        if(!r.success)
          res.push_back("Could not parse message: " + r.err_message);
        if(parser.get_string_parameter("ssid").length() > 0){
          _ssid = parser.get_string_parameter("ssid");
        }
        if(parser.get_string_parameter("pass").length() > 0){
          _pass = parser.get_string_parameter("pass");
        }
        if(parser.get_string_parameter("name").length() > 0){
          _name = parser.get_string_parameter("name");
        }
        if(parser.get_string_parameter("ssid").length() > 0 && parser.get_string_parameter("pass").length() > 0) {
          res.push_back("Network saved! ssid:" + String(_ssid) + (!String(_name).isEmpty() ? "name: " + String(_name) : ""));
          break;
        }
      }
    }

    if(_ssid != "" && _pass != ""){
      _ssid.toCharArray(ssid, 32);
      _pass.toCharArray(pass, 32);
      packet._name = _name;
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
