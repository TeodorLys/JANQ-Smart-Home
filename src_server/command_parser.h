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
	INIT_DEVICE
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

		int device_existance = devices.exists(device(last_mac, last_ip, last_name, ""));

		if(device_existance == file_handler::DOES_NOT_EXIST && last_req_type == "0"){
			//printf("was found\n");
			std::string req_form = net.get_request_form(last_ip);
			if(req_form != "NULL"){
				devices.add_new_device(device(last_mac, last_ip, last_name, req_form));
				log_printing(HIGH, "%cb[SYSINFO]%cw NEW DEVICE ON LAST REQUEST PARSE -> %s, REGISTERED\n", last_name.c_str());
			} else
				log_printing(HIGH, "%cy[SYSMESSAGE]%cw COULD NOT PARSE DEVICE REQUEST FORM, REGISTRATION -- %crFAILED%cw\n");
		}else if(device_existance == file_handler::SAME_INTERNAL_BUT_NOT_NAME){
			devices.change_name(device(last_mac, last_ip, last_name, ""), last_name);
		}

		if(last_status == "100" && last_req_type == "1"){
			if(last_message.find("192.168.0") != std::string::npos){
				formatted_message = last_message;
			}else {
				parse_command_line(last_message);
			}
		}else {
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
		}else if(last_status == "202"){
			current_code = STATUS_CODE_FAILED;
		}else if(last_status == "100") {
			current_code = REQUEST_CODE;
		}else if(last_status == "101"){
			current_code = DEVICE_INFO;
		}else if(last_status == "102"){
			current_code = INTERNAL_FUNCTION;
		}else if(last_status == "203"){
			current_code = INIT_DEVICE;
		}else{
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
		try{
			if(name.find(" ") != std::string::npos){
				name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
			}
			name.erase(name.find("{"), name.length());
			if(variables.find(" ") != std::string::npos){
				variables.erase(std::remove(variables.begin(), variables.end(), ' '), variables.end());
			}
			variables.erase(0, variables.find("{") + 1);
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
			log_printing(HIGH, "%cy[SYSMESSAGE]%cw could not parse device information, displaying all -> %s\n", e.what());
			return;
		}
		if(vars.size() == 0)
			return;
		file_handler file;
		device dev = file.find_device(name);
		std::string temp_format = dev._param;
		int index = 0;
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
};
