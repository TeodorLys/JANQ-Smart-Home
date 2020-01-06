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

String _header;
String assigned_name = "";
std::vector<parameter> params; // The current request parameters
std::vector<parameter> default_params; // The pre defined paramters
#ifdef ESP32
Preferences preference;
#endif
public:
  net_parser(bool _init = true) {
    if(_init){
      #ifdef ESP8266
      EEPROM.begin(20);
      String compare = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-_";
      for(int a = 0; a < 20; a++){
        char c;
        EEPROM.get(a, c);
        if(compare.indexOf(c) >= 0)
          assigned_name += c;
      }
      #else
      preference.begin("name", false);
      assigned_name = preference.getString("assigned", "UNASSIGNED");
      preference.end();
      #endif
      default_params.push_back(parameter("assign", 0));
      default_params.push_back(parameter("GET", 0));
    }
  }

  void _remote_init(){
    #ifdef ESP8266
      EEPROM.begin(20);
      String compare = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-_";
      for(int a = 0; a < 20; a++){
        char c;
        EEPROM.get(a, c);
        if(compare.indexOf(c) >= 0)
          assigned_name += c;
      }
      #else
      preference.begin("name", false);
      assigned_name = preference.getString("assigned", "UNASSIGNED");
      preference.end();
      #endif
      default_params.push_back(parameter("assign", 0));
      default_params.push_back(parameter("GET", 0));
  }

  void add_parameter(String p, int v = 0){
    default_params.push_back(parameter(p, v));
  }

  int parse_header(String header, void(*u)(String, int) = nullptr){
    if(header.indexOf("actionpage.php?") <= 0)
      return -1;
    params.clear();
    _header = header;
    if(_header.indexOf("?") > 0){
      _header.remove(0, _header.indexOf("?") + 1);
      for(int a = 0; a < default_params.size(); a++){
        extract_parameter_call();
        _header.remove(0, _header.indexOf("&") + 1);
      }
    }

    int ret_val = -1;

    /* If the request is a name change */
    String name = get_string_parameter("assign");

    /*
    A GET request is literally just getting information from the device,
    i.e. mac, ip, request form, name etc.
    */
    if(get_parameter("GET") != -1){
      if(u != nullptr)
        u("WHAT!", STATUS_DEVICE_INFO);
      ret_val = -2;
    }
    /*
    TODO: Move the name (re)assignment into a function.
    */
    if(name != "" && name != assigned_name){
      #ifdef ESP8266
      for(int a = 0; a < 20; a++){
        if(a < name.length())
          EEPROM.put(a, name[a]);
        else 
          EEPROM.put(a, 0);
      }
      EEPROM.commit();
      #else
      preference.begin("name", false);
      preference.putString("assigned", name);
      preference.end();
      #endif
      ret_val = -2;
      if(u != nullptr){
        String tosend = "NAME CHANGE REQUESTED, " + assigned_name;
        tosend += " -> " + name;
        u(tosend, STATUS_SUCCESS);
      }
      assigned_name = name;
    }else if(name == assigned_name){
      if(u != nullptr)
        u("NAME CHANGE REQUESTED, that is the same name...", STATUS_FAILED);
    }
    return ret_val;
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
       if(s._var_name != "assign" && s._var_name != "GET")
       buff += s._var_name + "=$&";
     }
     return buff;
   }

  void set_default_parameter(String name, int v){
    for(int a = 0; a < default_params.size(); a++){
      if(default_params[a]._var_name == name){
        default_params[a]._value = v;
        return;
      }
    }
  }

  //Hmm... why is this a pointer?
  String *get_assigned_name() {
    return &assigned_name;
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
   
private:
  void extract_parameter_call() {
    String s = _header;
    std::vector<int> param_array;
    /*
    Last parameter?
    */
		if (s.indexOf("&") > 0) {
			s.remove(s.indexOf("&"), s.length());
		}
		String name = s;
		if (s.indexOf("=") > 0) {
      name.remove(name.indexOf("="), name.length());

      s.remove(0, s.indexOf("=") + 1);
      if(s == ""){
        s = "0";
      /*
      The array request function was created for a TV remote, basically so you can
      send a timing array
      */
      } else if(s.indexOf("A") >= 0){
        s.remove(s.indexOf(" "), s.length());
        String buff;
        for(int a = 1; a < s.length() - 1; a++){
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
			try_without_equals(name, s);
		}

    if(name != ""){
      for(parameter ps : default_params){
        if(ps._var_name == name){
          params.push_back(parameter(name, s.toInt(), param_array, s));
        }
      }
    }
  }

  void try_without_equals(String &name, String &value) {
		String name_buff;
		String value_buff;
		for (char c : name) {
			if (char_is_num(c)) {
				value_buff += c;
			}
			else {
				name_buff += c;
			}
		}
		name = name_buff;
		if (value_buff.length() <= 0)
			value_buff = "0";
		value = value_buff;
	}

	bool char_is_num(char c) {
		String nums = "1234567890";
		return (nums.indexOf(c) >= 0);
	}
};