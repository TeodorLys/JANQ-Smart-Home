#pragma once
#include "udp_container.h"
#include <wiringPi.h>
#include <string>
#include <unistd.h>
#include "log_printing.h"

class internal_functions {
private:


public:
internal_functions() {
	wiringPiSetup();
	pinMode(15, OUTPUT);
}

void call_function(std::string f){
	if(f == "SCREEN_TOGGLE"){
		toggle_screen();
	}else if(f == "RESTART_SERVER"){
		restart();
	}else {
		log_printing(HIGH, "%cy[SYSMESSAGE]%cw THAT FUNCTION DOES NOT EXIST -- %crFAILED%cw");
	}
}

void toggle_screen(){
	digitalWrite(15, 1);
	delay(150);
	digitalWrite(15, 0);
}

void restart() {
//	const char args[] = { "/home/pi/udp_janq_command/main", 0 };
	udp_container::get_socket().unbind();
	execv("/home/pi/udp_janq_command/restart", 0);
	_exit(1);
}

};
