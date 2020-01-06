#include "udp_container.h"
#include <string>
#include "command_parser.h"
#include "net_request.h"
#include "log_printing.h"
#include "internal_functions.h"
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
//command_parser p;
//p.parse_command_line("DDDDDDDDDDDDDDDDDDDD{0,25,0}");
//net_request r;
std::string s;
log_printing(LOW, s, "%crhello%cw %s, %i, %f, %s, %i, %f\n", "world", 50, 55.5, "hello", 40, 44.5);
//r.get_connected_devices();
internal_functions functions;

if(udp_container::get_socket().bind(4123) != sf::Socket::Done){
	printf("Could not bind to port...\n");
	exit(0);
}

char data[200];
sf::IpAddress sender;
unsigned short port;
size_t recieved;

while(1){
	udp_container::get_socket().receive(data, 200, recieved, sender, port);
	std::string final_data;
	for(int a = 0; a < recieved; a++){
		final_data += data[a];
	}

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
			log_printing(HIGH, buffer, "[status: %s %s] INTERNAL FUNCTION CALLED -- %cgOK%cw\n", sender.toString().c_str(), format_time_stamp().c_str());
			functions.call_function(parser.get_data());
			file_handler::push_message_to_log(buffer);
		break;
		}

		case command_parser::UNKNOWN_STATUS_CODE:{
			std::string buffer;
			log_printing(HIGH, buffer, "%cy[SYSMESSAGE]%cw unknown status code, response unavailable! -- %crFAILED\n", sender.toString().c_str(), format_time_stamp().c_str());
			file_handler::push_message_to_log(buffer);
		break;
		}
		}
	}

}


}
