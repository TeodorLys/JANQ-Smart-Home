#pragma once
#include "udp_container.h"
#include <wiringPi.h>
#include <string>
#include <unistd.h>
#include "log_printing.h"
#include "device_handling/configure_device.h"

class internal_functions {
private:


public:
internal_functions() {
	std::string s = "sudo sh -c 'echo \"0\" > /sys/class/backlight/soc\\:backlight/brightness'";
	system(s.c_str());
	system("gpio -g mode 18 pwm");
	system("gpio pwmc 1000");
}

void call_function(std::string f){
	if(f == "SCREEN_TOGGLE"){
		toggle_screen();
	}else if(f == "RESTART_SERVER"){
		restart();
	}else if(f == "GET_ALL_LISTENERS"){
		get_all_listeners();
	}else if(f.find("register_status_listener") != std::string::npos) {
		try{
			std::string s = f;
			s.erase(0, s.find("(") + 1);
			if(s[s.size() - 1] == ')')
				s.pop_back();
			register_listener(s);
		}catch(std::exception &e){
			log_printing(HIGH, "%cy[SYSMESSAGE]%cw COULD NOT PARSE FUNCTION -- %crFAILED%cw\n");
			return;
		}
	}else if(f == "UNIZAP_UPDATE_AND_MAKE"){
    update_with_unizap();
  }else if(f.find("register_new_device") != std::string::npos) {
      std::string ssid = f;
      std::string pass;
      std::string name;
		try{
      ssid.erase(0, ssid.find("(") + 1);
      pass = ssid;
      ssid.erase(ssid.find(":"), ssid.length());
      pass.erase(0, pass.find(":") + 1);
      name = pass;
      pass.erase(pass.find(":"), pass.length());
      name.erase(0, name.find(":") + 1);
      name.erase(name.find(")"), name.length());
		}catch(std::exception &e){
			log_printing(HIGH, "%cy[SYSMESSAGE]%cw COULD NOT PARSE FUNCTION -- %crFAILED%cw\n");
			return;
		}
    configure_device device;
    device.search_for_new_device(ssid, pass, name);
	}
  else {
		log_printing(HIGH, "%cy[SYSMESSAGE]%cw THAT FUNCTION DOES NOT EXIST -- %crFAILED%cw\n");
  }
}

void get_all_listeners(){
	net_request req;
	req.send_all_listeners(true);
}

void register_listener(std::string s){
	if(!file_handler::find_listener(s))
		file_handler::register_status_listener(s);
}

void toggle_screen(){
	std::fstream file("/home/pi/udp_janq_command/gpio_commands/pin.state", std::fstream::in);
	std::string s;
	std::getline(file, s);
	printf("%s\n", s.c_str());
	file.close();
	if(file.is_open()){
		printf("still open...\n");
		file.close();
	}

	if(s == "0") {
		system("gpio -g pwm 18 1000");
		std::ofstream f("/home/pi/udp_janq_command/gpio_commands/pin.state");
		f << "1\n";
		f.close();
	}else{
		system("gpio -g pwm 18 0");
		std::ofstream f("/home/pi/udp_janq_command/gpio_commands/pin.state");
		if(!f.is_open())
			printf("Could not open file...\n");
		f << "0\n";
		f.close();
	}
}

void restart() {
//	const char args[] = { "/home/pi/udp_janq_command/main", 0 };
	udp_container::get_socket().unbind();
	udp_container::get_host_com().unbind();
  udp_container::get_forwarding_com().unbind();
	execv("/home/pi/udp_janq_command/src/restart", 0);
	_exit(1);
}

void update_with_unizap(){
  system("sudo sh /home/pi/unizap_helper/udp_helper.sh");
  restart();
}

};
