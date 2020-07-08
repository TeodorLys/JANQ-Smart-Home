#include "udp_container.h"
#include <string>
#include "command_parser.h"
#include "net_request.h"
#include "log_printing.h"
#include "internal_functions.h"
#include "conditions/condition.h"
#include <chrono>
#include <time.h>

std::string format_time_stamp() {
	auto end = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(end);
	std::string temp = std::ctime(&t);
	temp.pop_back();
	return temp;
}

int main() {
system("clear");
system("setterm -cursor off");

udp_container::bind_sockets();

std::string s;
log_printing(LOW, s, "%crhello%cw %s, %i, %f, %s, %i, %f\n", "world", 50, 55.5, "hello", 40, 44.5);
internal_functions functions;

char data[200];
sf::IpAddress sender;
unsigned short port;
size_t recieved;

while(1){
  net_request _net;
	udp_container::get_socket().receive(data, 200, recieved, sender, port);  
	std::string final_data;
	for(int a = 0; a < recieved; a++){
		final_data += data[a];
	}
  
  _net.forward_input(final_data);

	if(final_data.size() > 0){
		command_parser parser(final_data);
		switch(parser.get_last_code()){
		case command_parser::STATUS_CODE_SUCCESS:{
			std::string buffer;
			log_printing(HIGH, buffer, "[status: %s %s] %s -- %cgOK\n", sender.toString().c_str(), format_time_stamp().c_str(), parser.get_data().c_str());
			file_handler::push_message_to_log(buffer);
			//parser.print_packet();
		break;
		}
		case command_parser::STATUS_CODE_FAILED:{
			std::string buffer;
			log_printing(HIGH, buffer, "[status: %s %s] %s -- %crFAILED\n", sender.toString().c_str(), format_time_stamp().c_str(), parser.get_data().c_str());
			file_handler::push_message_to_log(buffer);
		break;
		}
		case command_parser::REQUEST_CODE:{
			std::string buffer;
			//log_printing(HIGH, buffer, "%cy[SYSMESSAGE]%cw %s\n", parser.get_data().c_str());
			file_handler::push_message_to_log(buffer);
			net_request req(parser.get_data());
		break;
		}

		case command_parser::DEVICE_INFO:{
			std::string buffer;
			log_printing(HIGH, buffer, "[status: %s %s] DEVICE INFO REQUESTED -- %cgOK\n", sender.toString().c_str(), format_time_stamp().c_str());
			parser.print_packet();
			file_handler::push_message_to_log(buffer);
		break;
		}
		case command_parser::INTERNAL_FUNCTION:{
			std::string buffer;
			log_printing(HIGH, buffer, "[status: %s %s] INTERNAL FUNCTION CALLED %s -- %cgOK%cw\n", sender.toString().c_str(), format_time_stamp().c_str(), parser.get_data().c_str());
			functions.call_function(parser.get_data());
			file_handler::push_message_to_log(buffer);
		break;
		}

		case command_parser::UNKNOWN_STATUS_CODE:{
			std::string buffer;
			log_printing(HIGH, buffer, "%cy[SYSMESSAGE]%cw unknown status code, response unavailable! -- %crFAILED%cw\n");
			file_handler::push_message_to_log(buffer);
		break;
		}
		case command_parser::STATUS_TO_HOST: {
			net_request req;
			//std::string buffer;
			//log_printing(HIGH, buffer, "[status %s %s] STATUS TO HOST, %s -- %cgOK%cw\n", sender.toString().c_str(), format_time_stamp().c_str(), parser.get_data().c_str());
			//file_handler::push_message_to_log(buffer);
			std::vector<listener> temp = parser.parse_status_to_hosts(parser.get_data());
			bool all_ok = true;
			for(int a = 0; a < temp.size(); a++){
				if(!file_handler::find_listener(temp[a].name)){
					all_ok = false;
				}else {
					file_handler::change_state_of_listener(temp[a].name, temp[a].state);
				}
			}
			if(all_ok){
				req.send_to_hosts(parser.get_data());
			}
		break;
		}
    /*
    TODO: This is very temporary! make a class to handle all sensor calls and register new actions
    */
    case command_parser::STATUS_SENSOR:{
      std::string buffer;
			log_printing(HIGH, buffer, "[status: %s %s] TEMP DOOR SENSOR -- %cgOK\n", sender.toString().c_str(), format_time_stamp().c_str());
			file_handler::push_message_to_log(buffer);
      net_request req;
      if(parser.get_data().find("1") != std::string::npos){
        req._remote_net_request("192.168.0.197/actionpage.php?Servo=000000", false);
        req._remote_net_request("192.168.0.101/actionpage.php?Servo=1023", false);
      }else if(parser.get_data().find("0") != std::string::npos){
        req._remote_net_request("192.168.0.197/actionpage.php?Servo=001500", false);
        req._remote_net_request("192.168.0.101/actionpage.php?Servo=0", false);
      }
    }
		default:
			log_printing(HIGH, "[status %s %s] REQUEST UNKNOWN - %s -- %crFAILED%cw\n", sender.toString().c_str(), format_time_stamp().c_str(), final_data.c_str());
		break;
		}
	}

}


}
