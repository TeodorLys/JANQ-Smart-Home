#pragma once
#include <curl/curl.h>
#include <string.h>
#include <iostream>

size_t write_data(void* ptr, size_t size, size_t nmemb, void* stream){
	((std::string*) stream)->append((const char*)ptr, (size_t)size * nmemb);
	return size * nmemb;
}

class net_request {
private:
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

	bool _remote_net_request(std::string req) {
		last_out = "";
		CURL* curl;
		CURLcode res = CURLE_OK;
		curl = curl_easy_init();
		if(curl){
			curl_easy_setopt(curl, CURLOPT_URL, req.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 500);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &last_out);
			res = curl_easy_perform(curl);
			if(res != CURLE_OK){
				log_printing(HIGH, "%cy[SYSMESSAGE]%cw CURL FAILED -> %s, %s\n", req.c_str(), curl_easy_strerror(res));
				curl_easy_cleanup(curl);
				return false;
			}
		}
		curl_easy_cleanup(curl);
		return true;
	}

	std::string get_request_form(std::string ip, bool display_error = true){
		std::string _req = "http://" + ip + "/"; //I know that you dont need to do this(http://)...
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

	void get_connected_devices() {
		for(int a = 2; a < 255; a++) {
			std::string ip_address = "192.168.0." + std::to_string(a);
			std::string req_form = get_request_form(ip_address, false);
			if(req_form != "NULL"){
				std::string temp = ip_address + req_form;
				ip_addresses.push_back(temp);
				printf("%s\n", temp.c_str());
			}
		}
	}
};
