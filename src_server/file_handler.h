#pragma once
#include "log_printing.h"
#include <fstream>
#include <string>
#include <vector>
#include <libconfig.h++>
#include <filesystem>

struct device {
	std::string _mac;
	std::string _ip;
	std::string _name;
	std::string _param;
	device(std::string mac, std::string ip, std::string name, std::string param) : _mac(mac), _ip(ip), _name(name), _param(param){}
};

struct listener {
	std::string name = "NIL";
	std::string state = "NULL";
	listener(std::string s, std::string status = "NULL") : name(s), state(status) {}
};

class file_handler {
public:
	enum existance {
		DOES_NOT_EXIST,
		DOES_EXIST,
		SAME_INTERNAL_BUT_NOT_NAME
	};

private:
	static std::vector<device> _devices;
	static std::vector<listener> _listeners;
	static std::string file_name;
	static libconfig::Config cfg;
	static std::fstream log;
	static bool first_launch;
public:
	file_handler() {
		if(!first_launch){
			return;
		}
			first_launch = false;
		try{
			cfg.readFile(file_name.c_str());
		}catch(const libconfig::FileIOException &e){
			if(!std::filesystem::exists(file_name)){
				libconfig::Setting& root = cfg.getRoot();
				root.add("net_devices", libconfig::Setting::TypeGroup);
				root.add("listeners", libconfig::Setting::TypeArray);
				root.add("states", libconfig::Setting::TypeArray);
				try{
					cfg.writeFile(file_name.c_str());
				}catch(libconfig::FileIOException& e){
					log_printing(HIGH, "%cy[SYSMESSAGE]%cwCould not write the save file... -- %crFAILED%cw\n");
				}
			}else{
				log_printing(HIGH, "%cy[SYSMESSAGE]%cwCould not read the save file... -- %crFAILED%cw\n");
			}
		}

		libconfig::Setting& root = cfg.getRoot();
		libconfig::Setting& net = root["net_devices"];
		if(!root.exists("listeners")){
			root.add("listeners", libconfig::Setting::TypeArray);
			try{
				cfg.writeFile(file_name.c_str());
			} catch(const libconfig::FileIOException &e) {
				log_printing(HIGH, "%cy[SYSMESSAGE]%cwCould not write the save file... -- %crFAILED%cw\n");
			}
		}
		if(!root.exists("states")){
			root.add("states", libconfig::Setting::TypeArray);
			try{
				cfg.writeFile(file_name.c_str());
			} catch(const libconfig::FileIOException &e) {
				log_printing(HIGH, "%cy[SYSMESSAGE]%cwCould not write the save file... -- %crFAILED%cw\n");
			}
		}
		libconfig::Setting& _list = root["listeners"];
		libconfig::Setting& _state = root["states"];
		for(int a = 0; a < net.getLength(); a++){
			_devices.push_back(device(net[a]["MAC"], net[a]["IP"], net[a].getName(), net[a]["PARAM"]));
		}

		for(int a = 0; a < _list.getLength(); a++){
			_listeners.push_back(listener(_list[a], _state[a]));
		}
	}

	static void register_status_listener(std::string s, std::string status = "NULL"){
		libconfig::Setting& root = cfg.getRoot();
		if(!root.exists("listeners")){
			root.add("listeners", libconfig::Setting::TypeArray);
		}
		if(!root.exists("states")){
			root.add("states", libconfig::Setting::TypeArray);
		}
		libconfig::Setting& _list = root["listeners"];
		libconfig::Setting& _state = root["states"];

		_list.add(libconfig::Setting::TypeString) = s;
		_state.add(libconfig::Setting::TypeString) = status;
		_listeners.push_back(listener(s, status));
		try{
			cfg.writeFile(file_name.c_str());
		} catch(const libconfig::FileIOException &e) {
			log_printing(HIGH, "%cy[SYSMESSAGE]%cwCould not write the save file... -- %crFAILED%cw");
		}
	}

	static void change_state_of_listener(std::string name, std::string status){
		libconfig::Setting& root = cfg.getRoot();
		if(!root.exists("listeners")){
			root.add("listeners", libconfig::Setting::TypeArray);
		}
		if(!root.exists("states")){
			root.add("states", libconfig::Setting::TypeArray);
		}
		libconfig::Setting& _list = root["listeners"];
		libconfig::Setting& _state = root["states"];
		//printf("Listeners: %s, %s\n", name.c_str(), status.c_str());

		for(int a = 0; a < _list.getLength(); a++){
			if(_list[a].c_str() == name){
				_state[a] = status;
			}
		}

		for(int a = 0; a < _listeners.size(); a++){
			if(_listeners[a].name == name){
				_listeners[a].state = status;
			}
		}

		try{
			cfg.writeFile(file_name.c_str());
		} catch(const libconfig::FileIOException &e) {
			log_printing(HIGH, "%cy[SYSMESSAGE]%cwCould not write the save file... -- %crFAILED%cw\n");
		}
	}

	void add_new_device(device dev) {
		libconfig::Setting& root = cfg.getRoot();
		libconfig::Setting& net = root["net_devices"];
		bool existed = true;
    //printf("after setting vars, %s\n", dev._name.c_str());
		if(!net.exists(dev._name))
			net.add(dev._name.c_str(), libconfig::Setting::TypeGroup);
    //printf("after devname vars\n");
		libconfig::Setting& address = net[dev._name.c_str()];
    //printf("after address vars\n");
		if(address.exists("MAC"))
			address["MAC"] = dev._mac;
		else{
			address.add("MAC", libconfig::Setting::TypeString) = dev._mac;
			existed = false;
		}
    //printf("mac\n");
		if(address.exists("IP"))
			address["IP"] = dev._ip;
		else{
			address.add("IP", libconfig::Setting::TypeString) = dev._ip;
			existed = false;
		}
    //printf("ip\n");
		if(address.exists("PARAM"))
			address["PARAM"] = dev._param;
		else{
			address.add("PARAM", libconfig::Setting::TypeString) = dev._param;
			existed = false;
		}
    //printf("after checking for exists\n");
		try{
			cfg.writeFile(file_name.c_str());
			if(!existed){
				_devices.push_back(dev);
			}else {
				for(int a = 0; a < _devices.size(); a++){
					if(_devices[a]._name == dev._name){
						_devices[a] = dev;
					}
				}
			}
		} catch(const libconfig::FileIOException &e) {
			log_printing(HIGH, "%cy[SYSMESSAGE]%cwCould not write the save file... -- %crFAILED%cw\n");
		}

	}

	void change_name(device dev, std::string _new) {
		libconfig::Setting& root = cfg.getRoot();
		libconfig::Setting& net = root["net_devices"];
		for(int a = 0; a < net.getLength(); a++){
			std::string __mac;
			net[a].lookupValue("MAC", __mac);
			if(__mac == dev._mac){
				net[a].lookupValue("PARAM", dev._param);
				net.remove(a);
				dev._name = _new;
				add_new_device(dev);
			}
		}

		for(int a = 0; a < _devices.size(); a++){
			if(_devices[a]._mac == dev._mac){
				_devices[a]._name = _new;
			}
		}

		try{
			cfg.writeFile(file_name.c_str());
		} catch(const libconfig::FileIOException &e) {
			log_printing(HIGH, "%cy[SYSMESSAGE]%cwCould not write the save file... -- %crFAILED%cw\n");
		}

	}

	void change_request_form(device dev, std::string _new) {
		libconfig::Setting& root = cfg.getRoot();
		libconfig::Setting& net = root["net_devices"];
		for(int a = 0; a < net.getLength(); a++){
			std::string __mac;
			net[a].lookupValue("MAC", __mac);
			if(__mac == dev._mac){
				net[a].lookupValue("PARAM", dev._param);
				net.remove(a);
				dev._param = _new;
				add_new_device(dev);
			}
		}

		for(int a = 0; a < _devices.size(); a++){
			if(_devices[a]._ip == dev._ip){
				_devices[a]._name = _new;
			}
		}

		try{
			cfg.writeFile(file_name.c_str());
		} catch(const libconfig::FileIOException &e) {
			log_printing(HIGH, "%cy[SYSMESSAGE]%cwCould not write the save file... -- %crFAILED%cw\n");
		}

	}

	static bool find_listener(std::string name){
		for(int a = 0; a < _listeners.size(); a++){
			if(_listeners[a].name == name)
				return true;
		}
		return false;
	}

	static std::string get_listeners(){
		std::string buffer;
		for(int a = 0; a < _listeners.size(); a++){
			buffer += _listeners[a].name + ":" + _listeners[a].state + "-";
		}
		return buffer;
	}

  static void push_message_to_log(std::string message){
    if(message == "")
      return;
		log.open("../log.txt", std::fstream::out | std::fstream::app);
		log << message;
		log << "\n";
		log.close();
	}

	int exists(device dev){
		for(int a = 0; a < _devices.size(); a++){
			if(_devices[a]._name == dev._name && _devices[a]._mac == dev._mac && _devices[a]._ip == dev._ip){
				return DOES_EXIST;
			}else if(_devices[a]._name != dev._name && _devices[a]._mac == dev._mac && _devices[a]._ip == dev._ip){
				return SAME_INTERNAL_BUT_NOT_NAME;
			}
		}
		//printf("no exists\n");
		return DOES_NOT_EXIST;
	}

	device find_device(std::string name){
		for(int a = 0; a < _devices.size(); a++){
			if(_devices[a]._name.find(name) != std::string::npos)
				return _devices[a];
		}
		log_printing(HIGH, "%cy[SYSMESSAGE]%cwDevice (%s) was not found!... -- %crFAILED%cw\n", name.c_str());
		return device("", "", "", "");
	}

};


std::fstream file_handler::log;
std::vector<device> file_handler::_devices;
std::vector<listener> file_handler::_listeners;
libconfig::Config file_handler::cfg;
std::string file_handler::file_name = "../devices.dev";
bool file_handler::first_launch = true;
