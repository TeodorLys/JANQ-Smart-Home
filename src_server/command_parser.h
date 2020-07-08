#pragma once
#include "file_handler.h"
#include "net_request.h"
#include "log_printing.h"

class command_parser {
public:
	enum code {
    UNKNOWN_STATUS_CODE,
    STATUS_CODE_SUCCESS,
    STATUS_CODE_FAILED,
    REQUEST_CODE,
    DEVICE_INFO,
    INTERNAL_FUNCTION,
    INIT_DEVICE,
    STATUS_TO_HOST,
    STATUS_SENSOR
	};

private:
	std::string data;
	std::string parsed_data;
	std::string last_mac;
	std::string last_ip;
	std::string last_status;
	std::string last_name;
	std::string last_req_type;  // if the requester is a host or slave
	std::string last_message;
	std::string formatted_message;
	file_handler devices;
	net_request net;
	code current_code;

public:


	command_parser(std::string fd) : data(fd){}
	command_parser() {}

	void parse_last(){
		bool failed = false;
		formatted_message = "";
		std::string temp = data;

		try{
			last_mac = data;
			temp.erase(0, temp.find(";") + 1);
			last_mac.erase(last_mac.find(";"), last_mac.length());
		}catch(std::exception &e){
			log_printing(LOW, "MAC %s\n", e.what());
			failed = true;
		}
		try{
			last_ip = temp;
			temp.erase(0, temp.find(";") + 1);
			last_ip.erase(last_ip.find(";"), last_ip.length());
		}catch(std::exception &e){
			log_printing(LOW, "IP %s\n", e.what());
			failed = true;
		}
		try{
			last_status = temp;
			temp.erase(0, temp.find(";") + 1);
			last_status.erase(last_status.find(";"), last_status.length());
		}catch(std::exception &e){
			log_printing(LOW, "STATUS %s\n", e.what());
			failed = true;
		}
		try{
			last_name = temp;
			temp.erase(0, temp.find(";") + 1);
			last_name.erase(last_name.find(";"), last_name.length());
		}catch(std::exception &e){
			log_printing(LOW, "NAME %s\n", e.what());
			failed = true;
		}
		try{
			last_req_type = temp;
			temp.erase(0, temp.find(";") + 1);
			last_req_type.erase(last_req_type.find(";"), last_req_type.length());

			last_message = temp;
		}catch(std::exception &e){
			log_printing(LOW, "REQ_TYPE %s\n", e.what());
			failed = true;
		}

		if(failed){
			log_printing(HIGH, "%cy[SYSMESSAGE]%cw could not parse device information, displaying all -> %s\n", data.c_str());
			last_message = data;
			return;
		}
    //printf("Checking for existance!\n");
		int device_existance = devices.exists(device(last_mac, last_ip, last_name, ""));

		if(device_existance == file_handler::DOES_NOT_EXIST && last_req_type == "0"){
      /*
      Looks up if the devices has a entry
      */
			std::string req_form = net.get_request_form(last_ip);
			if(req_form != "NULL") {
        //printf("Add new device!\n");        
				devices.add_new_device(device(last_mac, last_ip, last_name, req_form));
				log_printing(HIGH, "%cb[SYSINFO]%cw NEW DEVICE ON LAST REQUEST PARSE -> %s, REGISTRATION -- %cgOK%cw\n\n", last_name.c_str());
			} else
				log_printing(HIGH, "%cy[SYSMESSAGE]%cw COULD NOT PARSE DEVICE REQUEST FORM, REGISTRATION -- %crFAILED%cw\n");
		}else if(device_existance == file_handler::SAME_INTERNAL_BUT_NOT_NAME){
      //printf("Change name!\n");
			devices.change_name(device(last_mac, last_ip, last_name, ""), last_name);
		}

    /*
    Status code "100" are a host device wanting to communicate with a slave device.
    Like starting a light or open the blinds etc.
    */
		if(last_status == "100" && last_req_type == "1"){
			if(last_message.find("192.168") != std::string::npos){
				formatted_message = last_message;
			}else {
				parse_command_line(last_message);
			}
		}else {
      /*
      A slave devices sends some information when it starts. So "203" status code
      are when a slave sends its request form and name.
      */
			if(last_req_type == "203"){
				std::string req_form = net.get_request_form(last_ip);
				device d = devices.find_device(last_name);
				if(d._param != req_form);
					devices.change_request_form(d, req_form);
			}
			formatted_message = last_message;
		}

	}

	code get_last_code() {
		parse_last();
		if(last_status == "200") {
			current_code = STATUS_CODE_SUCCESS;
		} else if(last_status == "202"){
			current_code = STATUS_CODE_FAILED;
		} else if(last_status == "100") {
			current_code = REQUEST_CODE;
		} else if(last_status == "101"){
			current_code = DEVICE_INFO;
		} else if(last_status == "102"){
			current_code = INTERNAL_FUNCTION;
		} else if(last_status == "203"){
			current_code = INIT_DEVICE;
		} else if(last_status == "204"){
			current_code = STATUS_TO_HOST;
    } else if(last_status == "205"){
			current_code = STATUS_SENSOR;
		} else {
			current_code = UNKNOWN_STATUS_CODE;
		}
		return current_code;
	}

	void print_packet(){
		log_printing(HIGH, "{\nMAC:%s\nIP:%s\nSTATUS:%s\nNAME:%s\n}\n", last_mac.c_str(), last_ip.c_str(), last_status.c_str(), last_name.c_str());
	}

	std::string get_data() {
		return formatted_message;
	}

	void parse_command_line(std::string s){
		std::string name = s;
		std::string variables = s;
		std::vector<std::string> vars;
    /*Parses ex. stringlight{0,001500} into something readable*/
		try{
      /*ex. stringlight {0, 001500} removes the whitespaces!
                       ^   ^
      */
			if(name.find(" ") != std::string::npos){
				name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
			}
      /*removes ex. stringlight{0,001500} to get the name of the device
                               xxxxxxxxx^*/
			name.erase(name.find("{"), name.length());
      /*SEE above "name.find(" ")"*/
			if(variables.find(" ") != std::string::npos){
				variables.erase(std::remove(variables.begin(), variables.end(), ' '), variables.end());
			}
      /*removes ex. stringlight{0,001500} to get the parameters
                    xxxxxxxxxxx^*/
			variables.erase(0, variables.find("{") + 1);
      /*removes ex. stringlight{0,001500}
                                        ^*/
			variables.pop_back();
			std::string t;
			for(int a = 0; a < variables.size(); a++){
				if(variables[a] != ','){
					t += variables[a];
				}else {
					vars.push_back(t);
					t = "";
				}
			}
			vars.push_back(t);
			t = "";
		}catch(std::exception &e){
			log_printing(HIGH, "%cy[SYSMESSAGE]%cw could not parse device information, displaying all -> %s\n", s.c_str());
			return;
		}
    /*If no parameters was found in request*/
		if(vars.size() == 0)
			return;

		file_handler file;
		device dev = file.find_device(name);
    /*Default request form from device*/
		std::string temp_format = dev._param;
		int index = 0;
    /*Replaces the $ with the parsed parameters from request. 
    if one of the parameters are unfilled it will be replaced with a 0*/
		for(int a = 0; a < temp_format.size(); a++){
			if(temp_format[a] == '$' && index < vars.size()){
				temp_format.replace(temp_format.begin() + a, temp_format.begin() + a + 1, vars[index]);
				a += vars[index].size();
				index++;
			}else if(temp_format[a] == '$'){
				temp_format.replace(temp_format.begin() + a, temp_format.begin() + a + 1, "0");
			}
		}
		formatted_message = dev._ip + temp_format;
		log_printing(LOW, "formatted: %s\n", formatted_message.c_str());
	}

	std::vector<listener> parse_status_to_hosts(std::string message){
		std::vector<listener> temp;
		std::string buffer;
		if(message[message.size() - 1] != '-')
			message += '-';
		try{
			for(int a = 0; a < message.size(); a++){
				if(message[a] != '-'){
					buffer += message[a];
				}else {
					std::string m;
					std::string s;
					m = buffer;
					m.erase(m.find(":"), m.length());
					s = buffer;
					s.erase(0, s.find(":") + 1);
					temp.push_back(listener(m, s));
					buffer = "";
				}
			}
		}catch(std::exception &e){
			log_printing(HIGH, "%cy[SYSMESSAGE]%cw could not parse status to hosts, exiting... -> %s\n", e.what());
			return temp;
		}
		return temp;
	}

};
