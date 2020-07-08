#pragma once
#include <curl/curl.h>
#include <string.h>
#include <iostream>
#include <SFML/System/Clock.hpp>
#include "udp_container.h"
#include "file_handler.h"

size_t write_data(void* ptr, size_t size, size_t nmemb, void* stream){
	((std::string*) stream)->append((const char*)ptr, (size_t)size * nmemb);
	return size * nmemb;
}

class net_request {
private:
	sf::Clock clock;
	std::string last_out;
	std::vector<std::string> ip_addresses;
public:
	net_request() {}

	net_request(std::string req) {
		CURL* curl;
		CURLcode res = CURLE_OK;
		curl = curl_easy_init();
		if(curl){
			curl_easy_setopt(curl, CURLOPT_URL, req.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &last_out);
			res = curl_easy_perform(curl);
			if(res != CURLE_OK){
				log_printing(HIGH, "%cy[SYSMESSAGE]%cw CURL FAILED -> %s, %s\n", req.c_str(), curl_easy_strerror(res));
			}
		}
		curl_easy_cleanup(curl);
	}

	bool _remote_net_request(std::string req, bool print = true) {
		last_out = "";
		CURL* curl;
		CURLcode res = CURLE_OK;
		curl = curl_easy_init();
		if(curl){
			curl_easy_setopt(curl, CURLOPT_URL, req.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1500);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &last_out);
			res = curl_easy_perform(curl);
			if(res != CURLE_OK){
        if(print)
				  log_printing(HIGH, "%cy[SYSMESSAGE]%cw CURL FAILED -> %s, %s\n", req.c_str(), curl_easy_strerror(res));
				curl_easy_cleanup(curl);
        if(last_out == "")
				  return false;
        else 
          return true;
			}
		}
		curl_easy_cleanup(curl);
		return true;
	}

	std::string get_request_form(std::string ip, bool display_error = true){
		std::string _req = "http://" + ip + "/";
		std::string form = "NULL";
		if(!_remote_net_request(_req)){
			if(display_error){
				log_printing(HIGH, "%cy[SYSMESSAGE]%cw IP ADDRESS WAS INVALID! -> %s -- %crFAILED%cw\n", _req.c_str());
			}
			return form;
		}
		try{
			form = last_out;
			form.erase(0, form.find("requestform"));
			form.erase(form.find(">") - 1, form.length());
			form.erase(0, form.find("/"));
		}catch(std::exception &e){
			log_printing(HIGH, "%cy[SYSMESSAGE]%cw PARSE OF THE REQUEST FORM WAS UNSUCCESSFUL -> %s -- %crFAILED%cw\n%s\n", e.what(), ip.c_str());
			form = "NULL";
		}
		return form;
	}


  /*
  TODO: Make this send stuff to all hosts, not just my phone!
  */
	void send_to_hosts(std::string s, bool skip_clock = false){
		std::string to_send = format_packet_form() + s;
		//printf("%s\n", to_send.c_str());
		if(clock.getElapsedTime().asSeconds() >= 30 || skip_clock){
      unsigned short port = 4133;
			if(udp_container::get_host_com().send(to_send.c_str(), to_send.length(), "192.168.86.33", port)){
				log_printing(HIGH, "%cr[SYSERROR]%cw Could not send through UDP 4133 -- %crFAILED%cw\n");
			}else {
        if(skip_clock)
          log_printing(HIGH, "%cy[SYSMESSAGE]%cw Sent %s to 4133 -- %cgOK%cw\n", s.c_str());
      }
			clock.restart();
		}
	}

  void forward_input(std::string to_send){
    if(udp_container::get_host_com().send(to_send.c_str(), to_send.length(), "255.255.255.255", 6123)){
				log_printing(HIGH, "%cr[SYSERROR]%cw Could not send through UDP 4133 -- %crFAILED%cw\n");
    }
  }

	std::string format_packet_form(){
		std::string s = "192.168.86.241";
		s += ";00:00:00:00:00;";
		s += "204;JANQ_Server;1;";
		return s;
	}

  void rebind_host_socket(){
    udp_container::get_host_com().unbind();
    if(udp_container::get_host_com().bind(4133) != sf::Socket::Done){
	    printf("Could not bind to host port...\n");
	    exit(0);
    } 
  }

	void send_all_listeners(bool skip_clock = false){
		std::string listeners = file_handler::get_listeners();
		//printf("%s\n", listeners.c_str());
		send_to_hosts(listeners, skip_clock);
	}

	void get_connected_devices() {
		for(int a = 2; a < 255; a++) {
			std::string ip_address = "192.168.0." + std::to_string(a);
			std::string req_form = get_request_form(ip_address, false);
			if(req_form != "NULL"){
				std::string temp = ip_address + req_form;
				ip_addresses.push_back(temp);
				//printf("%s\n", temp.c_str());
			}
		}
	}
};
