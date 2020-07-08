#pragma once
#include <vector>
#include "Status_codes.h"
#ifdef ESP8266
#include <EEPROM.h>
#else 
#include <Preferences.h>
#endif

/*
TODO:
Make this apart of the net_connection class.
*/


struct _return {
  bool assign = false;
  bool request_listener = false;
  bool GET = false;
  bool success = false;
  String err_message = "";
};

class net_parser{
private:

struct parameter{
  String _var_name;
  int _value;
  String _internal_value;
  std::vector<int> param_array;
  parameter(String var_name, int value, std::vector<int> array = std::vector<int>(), String _internal_ = "") : _var_name(var_name), _value(value){
    param_array = array;
    _internal_value = _internal_;
  }
};

struct listener{
  String name;
  String state;
  listener(String _name, String _state) : name(_name), state(_state) {}
};

String _header;
String assigned_name = "";
std::vector<parameter> params; // The current request parameters
std::vector<parameter> default_params; // The pre defined paramters
std::vector<listener> listeners;
#ifdef ESP32
Preferences preference;
#endif
public:
  net_parser() {
    /*
    These parameters are lib defaults, and are used for janq server stuff.
    *NOTE* These are NOT included in the request form! SEE: "get_formatted_request_form" function
    */
    default_params.push_back(parameter("assign", 0));
    default_params.push_back(parameter("GET", 0));
    default_params.push_back(parameter("request_listener", 0));
  }

  void register_local_listener(String name, String default_state = "NULL"){
    listeners.push_back(listener(name, default_state));
  }

  void change_local_listener(String name, String state, void(*u)(String, int)){
    for(int a = 0; a < (int)listeners.size(); a++){
      if(listeners[a].name == name){
        listeners[a].state = state;
        u(_format_request_listener(), STATUS_TO_HOSTS);
        break;
      }
    }
  }

  void _remote_init() {
      default_params.push_back(parameter("assign", 0));
      default_params.push_back(parameter("GET", 0));
      default_params.push_back(parameter("request_listener", 0));
  }

  void add_parameter(String p, int v = 0){
    default_params.push_back(parameter(p, v));
  }

  void clear_buffer(){
    for(int a = 0; a < (int)params.size(); a++){
      params[a]._var_name = "";
      params[a]._internal_value = "";
      params[a]._value = 0;
    }
    params.clear();
  }

  _return parse_header(String header, void(*u)(String, int) = nullptr){
    _return ret;
    if(header.indexOf("actionpage.php?") <= 0){
      ret.err_message = "no actionpage.php?";
      return ret;
    }

    params.clear();
    _header = header;
    if(_header.indexOf("?") > 0){
      _header.remove(0, _header.indexOf("?") + 1);
      for(int a = 0; a < (int)default_params.size(); a++){
        _return __r = extract_parameter_call();
        if(!__r.success){
          ret.err_message = "extract call failed: " + __r.err_message;
          return ret;
        }
        _header.remove(0, _header.indexOf("&") + 1);
      }
    }

    ret.success = true;

    /* If the request is a name change */
    String name = get_string_parameter("assign");

    /*
    A GET request is literally just getting information from the device,
    i.e. mac, ip, request form, name etc.
    */
    if(get_parameter("GET") != -1){
      if(u != nullptr)
        u("WHAT!", STATUS_DEVICE_INFO);
      ret.GET = true;
    }else if(get_parameter("request_listener") != -1){
      if(u != nullptr)
        u(_format_request_listener(), STATUS_TO_HOSTS);
      ret.request_listener = true;
    }

    if(name != "" && name != assigned_name){
      if(u != nullptr)
        u("NAME CHANGE REQUESTED", STATUS_SUCCESS);
      assigned_name = name;
      ret.assign = true;
    }else if(name == assigned_name){
      if(u != nullptr)
        u("NAME CHANGE REQUESTED, that is the same name...", STATUS_FAILED);
    }
    return ret;
  }

  /*
  Returns a specified parameter as a int. If the parameter is a string,
  like a function call or a name assign, it will be returned as a 0.
  if a parameter isnt included in the request this will return -1;
  */
  int get_parameter(String name) {
    for(parameter p : params){
      if(name == p._var_name){
        return p._value;
      }
    }
    return -1;
  }

  std::vector<int> get_parameter_array(String name) {
    for(parameter p : params){
      if(name == p._var_name){
        return p.param_array;
      }
    }
    return std::vector<int>(0);
  }

  String get_formatted_request_form() {
    String buff = "/actionpage.php?";
    for(parameter s : default_params){
      if(s._var_name != "assign" && s._var_name != "GET" && s._var_name != "request_listener")
      buff += s._var_name + "=$&";
    }
    return buff;
  }

  void set_default_parameter(String name, int v){
    for(int a = 0; a < (int)default_params.size(); a++){
      if(default_params[a]._var_name == name){
        default_params[a]._value = v;
        return;
      }
    }
  }

  void set_assigned_name(String name){
    assigned_name = name;
  }

  String get_assigned_name() {
    return assigned_name;
  }
  /*
  Return a specified parameter in string format. Some 
  paramters is requested as string, like a function call or name assign.
  */
  String get_string_parameter(String name) {
     for(parameter p : params){
       if(name == p._var_name){
         return p._internal_value;
       }
     }
     return "";
  }

  bool get_parameter_exists(String name) {
     for(parameter p : params){
       if(name == p._var_name){
         return true;
       }
     }
     return false;
  }

   
private:
  _return extract_parameter_call() {
    _return ret;
    String s = _header;
    std::vector<int> param_array;
    /*
    Last parameter?
    */
		if (s.indexOf("&") > 0) {
			s.remove(s.indexOf("&"));
		}
		String name = s;
		if (s.indexOf("=") > 0) {
      name.remove(name.indexOf("="));

      s.remove(0, s.indexOf("=") + 1);
      if(s == ""){
        s = "0";
      } 
      /*
      The array request function was created for a TV remote, basically so you can
      send a timing array
      */
      else if(s.indexOf("A") >= 0){
        s.remove(s.indexOf(" "));
        String buff;
        for(int a = 1; a < (int)s.length() - 1; a++){
          if(s[a] != '+'){
            buff += s[a];
          }else if(s[a] == '+'){
            param_array.push_back(buff.toInt());
            buff = "";
          }
        }
      }
    }
		else {
      /*
      If a "=" is forgotten in the request this function will
      try to get a value from it by looking for a int, and/or by looking for the 
      defined parameter and what is after it. 
      */
			if(try_without_equals(name, s)){
        ret.err_message = "try without equals failed";
        return ret;
      }
		}

    if(name != ""){
      for(parameter ps : default_params){
        if(ps._var_name == name){
          params.push_back(parameter(name, s.toInt(), param_array, s));
          ret.success = true;
          return ret;
        }
      }
      ret.err_message = "Parameter was not found: " + name;
      return ret;
    }
    return ret;
  }

  String _format_request_listener(){
    String buffer;
    for(int a = 0; a < (int)listeners.size(); a++){
      buffer += listeners[a].name + ":" + listeners[a].state + "-";
    }
    return buffer;
  }

  bool try_without_equals(String &name, String &value) {
		String name_buff;
		String value_buff;
    for(parameter p : default_params){
      if(name.indexOf(p._var_name) >= 0){
        name_buff = name;
        name_buff.remove(name_buff.indexOf(p._var_name) + p._var_name.length()); 
        value_buff = name;
        value_buff.remove(0, name_buff.indexOf(p._var_name) + p._var_name.length());
        name = name_buff;
        value = value_buff;
        return true;
      }
    }
    return false;
	}

	bool char_is_num(char c) {
		String nums = "1234567890";
		return (nums.indexOf(c) >= 0);
	}
};