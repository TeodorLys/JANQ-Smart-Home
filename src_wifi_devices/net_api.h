#include "Net_Connection.h"
#include "Net_Parser.h"
#include <cstdarg>

#define GET_STRING true
#define GET_INT 'a'
#define GET_ARRAY (String)"array"
#define GET_EXISTANCE 4.7 // My favorite number!

class net_api{
private:
  net_parser *parser;
  net_connection *net;
  String __ssid = "";
  String __pass = "";
  class response : public response_handler {
  public:
    void client_print(WiFiClient &client) {}
  };
  void(*c_func)();
  bool _enable_mdns = false;

  response res;
  String p[20];
public:
  net_api(){
    parser = new net_parser();
    net = new net_connection(&res);
    for(int a = 0; a < 20; a++)
      p[a] = "0";
  }

  void set_connect_function(void(*func)()){
    c_func = func;
  }

  void set_wifi_credentials(String _ssid, String _pass){
    __ssid = _ssid;
    __pass = _pass;
  }

  void initialize(int param_count, const char* c, ...){
    va_list list;
    va_start(list, c);
    parser->add_parameter(c);
    if(param_count > 20)
      param_count = 20;
    for(int a = 0; a < param_count - 1; a++){
      const char* s = va_arg(list, const char*);
      parser->add_parameter(s);
      p[a] = s;
    }
    va_end(list);
    if(_enable_mdns)
      net->enable_mdns();
    net->set_request_form(parser->get_formatted_request_form());
    net->connect(__ssid, __pass, c_func);
    parser->set_assigned_name(net->get_name());
  }

  bool got_request(){
    parser->clear_buffer();
    if(net->new_client_connected()){
      _return r = parser->parse_header(net->get_header(), &send_status_to_server);
      if(!r.success){
        net->send_status_to_server("Could not parse request message", STATUS_FAILED);
        return false;
      }
      if(r.assign){
        net->set_normal_name(parser->get_assigned_name());
        net->saveCredentials();
      }
      return true;
    }
    return false;
  }

  void parameter(String name, int default_value = 0){
    parser->add_parameter(name, default_value);
  }

  bool parameter(String name, float f){
    return parser->get_parameter_exists(name);
  }

  int parameter(String name, char get_int) {
    return parser->get_parameter(name);
  }

  String parameter(String name, bool get_string) {
    return parser->get_string_parameter(name);
  }

  std::vector<int> parameter(String name, String arr){
    return parser->get_parameter_array(name);
  }

  /*
  TODO: This only sets the parser name and remove the if statement... it acts like a one way valve!
  */

  void set_device_name(String name){
    if(parser->get_assigned_name() == "" || parser->get_assigned_name() == "UNASSIGNED")
      parser->set_assigned_name(name);
  }

  String device_name() const {
    return net->get_name();
  }

  void disable_static(){
    net->disable_static();
  }

  void enable_mdns(){
    _enable_mdns = true;
  }

  void push_message_to_server(String s, int status = 200){
    net->send_status_to_server(s, status);
  }

};