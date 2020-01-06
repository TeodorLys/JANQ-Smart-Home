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

class file_handler {
public:
	enum existance {
		DOES_NOT_EXIST,
		DOES_EXIST,
		SAME_INTERNAL_BUT_NOT_NAME
	};

private:
	std::vector<device> _devices;
	const std::string file_name = "devices.dev";
	libconfig::Config cfg;
	static std::fstream log;
public:
	file_handler() {
		try{
			cfg.readFile(file_name.c_str());
		}catch(const libconfig::FileIOException &e){
			if(!std::filesystem::exists(file_name)){
				libconfig::Setting& root = cfg.getRoot();
				root.add("net_devices", libconfig::Setting::TypeGroup);
				try{
					cfg.writeFile(file_name.c_str());
				}catch(libconfig::FileIOException& e){
					log_printing(HIGH, "%cy[SYSMESSAGE]%cwCould not write the save file... -- %crFAILED%cw");
				}
			}else{
				log_printing(HIGH, "%cy[SYSMESSAGE]%cwCould not read the save file... -- %crFAILED%cw");
			}
		}

		libconfig::Setting& root = cfg.getRoot();
		libconfig::Setting& net = root["net_devices"];

		for(int a = 0; a < net.getLength(); a++){
			_devices.push_back(device(net[a]["MAC"], net[a]["IP"], net[a].getName(), net[a]["PARAM"]));
		}
	}

	void add_new_device(device dev) {
		libconfig::Setting& root = cfg.getRoot();
		libconfig::Setting& net = root["net_devices"];
		bool existed = true;

		if(!net.exists(dev._name))
			net.add(dev._name.c_str(), libconfig::Setting::TypeGroup);

		libconfig::Setting& address = net[dev._name.c_str()];

		if(address.exists("MAC"))
			address["MAC"] = dev._mac;
		else{
			address.add("MAC", libconfig::Setting::TypeString) = dev._mac;
			existed = false;
		}

		if(address.exists("IP"))
			address["IP"] = dev._ip;
		else{
			address.add("IP", libconfig::Setting::TypeString) = dev._ip;
			existed = false;
		}

		if(address.exists("PARAM"))
			address["PARAM"] = dev._param;
		else{
			address.add("PARAM", libconfig::Setting::TypeString) = dev._param;
			existed = false;
		}

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
			log_printing(HIGH, "%cy[SYSMESSAGE]%cwCould not write the save file... -- %crFAILED%cw");
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
			if(_devices[a]._ip == dev._ip){
				_devices[a]._name = _new;
			}
		}

		try{
			cfg.writeFile(file_name.c_str());
		} catch(const libconfig::FileIOException &e) {
			log_printing(HIGH, "%cy[SYSMESSAGE]%cwCould not write the save file... -- %crFAILED%cw");
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
			log_printing(HIGH, "%cy[SYSMESSAGE]%cwCould not write the save file... -- %crFAILED%cw");
		}

	}

        static void push_message_to_log(std::string message){
		log.open("log.txt", std::fstream::out | std::fstream::app);
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
			if(_devices[a]._name == name)
				return _devices[a];
		}
		log_printing(HIGH, "%cy[SYSMESSAGE]%cwDevice was not found!... -- %crFAILED%cw");
		return device("", "", "", "");
	}

};


std::fstream file_handler::log;
