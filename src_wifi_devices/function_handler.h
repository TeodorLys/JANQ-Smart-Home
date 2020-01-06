#pragma once

#include <Arduino.h>
#include "Net_Connection.h"
#include "Status_codes.h"

/*
TODO:
Make this apart of the net_connection class.
*/

template<int T_param>
class function_handler {
private:
  bool is_active = false;
  void (*f)();
public:
//TODO: get this in a function call!
  int p[T_param];

  void register_function(void (*_func)()){
    f = _func;
  }

  void parse_function_call(String call){
    String _name = call;
    _name.remove(_name.indexOf("{"));
    String params = call;
    params.remove(0, params.indexOf("{") + 1);
    int index = 0;
    int loop_index = 0;
    String temp;
    /*
    Parse the function parameters with the set amount of parameters in the template(T_param).
    TODO: Change the function call syntax. It is really unintuitive i.e. function{p1:p2:p3},
    really stupid!

    Also, the paramaters are always an integer!
    */
    while(params[index] != '}'){
      if(loop_index >= T_param) {
        send_status_to_server("FUNCTION CALL HAD TOO MANY PARAMETERS, EXITING!", STATUS_FAILED);
      }
      if(params[index] != ':')
        temp += params[index];
      else if(params[index] == ':'){
        p[loop_index] = temp.toInt();
        temp = "";
        loop_index++;
      }
      index++;
    }
    p[loop_index] = temp.toInt();
  }

void call_function(){
  f();
}

void active(bool b){
  is_active = b;
}

bool active() const {
  return is_active;
}

};