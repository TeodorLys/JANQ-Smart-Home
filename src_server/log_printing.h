#pragma once

#include <stdio.h>
#include <cstdarg>
#include <string>
#include <vector>


enum {
	LOW,
	MEDIUM,
	HIGH
};

class log_printing {
private:
struct variables {
	int _start_index;
	std::string _type;
	bool _if_color;
	variables(int start_index, std::string type, bool if_color = false) : _start_index(start_index), _type(type), _if_color(if_color) {}
};

const int print_type = MEDIUM;

const std::string RED = "\e[31m";
const std::string GREEN = "\e[32m";
const std::string YELLOW = "\e[33m";
const std::string BLUE = "\e[34m";
const std::string DEFAULT = "\e[39m";

const std::vector<char> var_types = {'f', 's', 'i'};
const std::vector<char> color_types = {'r', 'g', 'b', 'y', 'w'};
std::vector<variables> parsed_types;

public:

log_printing(int type, std::string &s, std::string format...){

	for(int a = 0; a < format.length(); a++){
		if(format[a] == '%'){
			switch(format[a + 1]) {
				case 'c':
					color_parse(format, a + 1);
				break;
				default:
					default_var_parse(format, a + 1);
				break;
			}
		}
	}

	va_list args;
	va_start(args, format);

	std::string log_buffer = format, buffer = format;

	std::vector<variables> color_parsed = parsed_types;

	for(int a = 0; a < parsed_types.size(); a++){
		if(!parsed_types[a]._if_color){
			if(parsed_types[a]._type == "f"){
				double d = va_arg(args, double);
				std::string conv = std::to_string(d);
				//printf("%s, %s", conv.c_str(), buffer.c_str());
				log_buffer.replace(color_parsed[a]._start_index, 2, conv);
				buffer.replace(parsed_types[a]._start_index, 2, conv);
				if(a + 1 < parsed_types.size())
					for(int b = a; b < parsed_types.size(); b++){
						parsed_types[b]._start_index += conv.size() - 2;
						color_parsed[b]._start_index += conv.size() - 2;
					}
			}
			else if(parsed_types[a]._type == "s"){
				const char *s = va_arg(args, const char*);
				std::string conv = s;
				//printf("%s, %s", conv.c_str(), buffer.c_str());
				log_buffer.replace(color_parsed[a]._start_index, 2, s);
				buffer.replace(log_printing::parsed_types[a]._start_index, 2, s);
				if(a + 1 < parsed_types.size())
					for(int b = a; b < parsed_types.size(); b++){
						log_printing::parsed_types[b]._start_index += conv.size() - 2;
						color_parsed[b]._start_index += conv.size() - 2;
					}
			}
			else if(parsed_types[a]._type == "i"){
				int i = va_arg(args, int);
				std::string conv = std::to_string(i);
				//printf("%s, %s", conv.c_str(), buffer.c_str());
				log_buffer.replace(color_parsed[a]._start_index, 2, std::to_string(i));
				buffer.replace(parsed_types[a]._start_index, 2, std::to_string(i));
				if(a + 1 < parsed_types.size())
					for(int b = a; b < parsed_types.size(); b++){
						parsed_types[b]._start_index += conv.size() - 2;
						color_parsed[b]._start_index += conv.size() - 2;
					}
			}
		} else {
			//printf("%s\n", parsed_types[a]._type.c_str());
			buffer.replace(parsed_types[a]._start_index, 3, parsed_types[a]._type);
			log_buffer.erase(color_parsed[a]._start_index, 3);
			for(int b = a + 1; b < parsed_types.size(); b++){
				parsed_types[b]._start_index += parsed_types[a]._type.size() - 3;
				color_parsed[b]._start_index -= 3;
			}
		}
	}
	buffer += DEFAULT;
	if(type >= print_type){
		printf("%s", buffer.c_str());
	}
	s = log_buffer;
}


log_printing(int type, std::string format...){
	if(type < print_type){
		return;
	}
	for(int a = 0; a < format.length(); a++){
		if(format[a] == '%'){
			switch(format[a + 1]) {
				case 'c':
					color_parse(format, a + 1);
				break;
				default:
					default_var_parse(format, a + 1);
				break;
			}
		}
	}

	va_list args;
	va_start(args, format);

	std::string log_buffer = format, buffer = format;

	std::vector<variables> color_parsed = parsed_types;
	for(int a = 0; a < parsed_types.size(); a++){
		if(!parsed_types[a]._if_color){
			if(parsed_types[a]._type == "f"){
				double d = va_arg(args, double);
				std::string conv = std::to_string(d);
				//printf("%s, %s", conv.c_str(), buffer.c_str());
				log_buffer.replace(color_parsed[a]._start_index, 2, conv);
				buffer.replace(parsed_types[a]._start_index, 2, conv);
				if(a + 1 < parsed_types.size())
					for(int b = a; b < parsed_types.size(); b++){
						parsed_types[b]._start_index += conv.size() - 2;
						color_parsed[b]._start_index += conv.size() - 2;
					}
			}
			else if(parsed_types[a]._type == "s"){
				const char *s = va_arg(args, const char*);
				std::string conv = s;
				//printf("%s, %s", conv.c_str(), buffer.c_str());
				log_buffer.replace(color_parsed[a]._start_index, 2, s);
				buffer.replace(log_printing::parsed_types[a]._start_index, 2, s);
				if(a + 1 < parsed_types.size())
					for(int b = a; b < parsed_types.size(); b++){
						log_printing::parsed_types[b]._start_index += conv.size() - 2;
						color_parsed[b]._start_index += conv.size() - 2;
					}
			}
			else if(parsed_types[a]._type == "i"){
				int i = va_arg(args, int);
				std::string conv = std::to_string(i);
				//printf("%s, %s", conv.c_str(), buffer.c_str());
				log_buffer.replace(color_parsed[a]._start_index, 2, std::to_string(i));
				buffer.replace(parsed_types[a]._start_index, 2, std::to_string(i));
				if(a + 1 < parsed_types.size())
					for(int b = a; b < parsed_types.size(); b++){
						parsed_types[b]._start_index += conv.size() - 2;
						color_parsed[b]._start_index += conv.size() - 2;
					}
			}
		} else {
			//printf("%s\n", parsed_types[a]._type.c_str());
			buffer.replace(parsed_types[a]._start_index, 3, parsed_types[a]._type);
			log_buffer.erase(color_parsed[a]._start_index, 3);
			for(int b = a + 1; b < parsed_types.size(); b++){
				parsed_types[b]._start_index += parsed_types[a]._type.size() - 3;
				color_parsed[b]._start_index -= 3;
			}
		}
	}
	buffer += DEFAULT;
	printf("%s", buffer.c_str());

}

protected:

void default_var_parse(std::string format, int index){
	for(int a = 0; a < var_types.size(); a++){
		if(format[index] == var_types[a]){
			std::string s(1, var_types[a]);
			parsed_types.push_back(variables(index - 1, s));
		}
	}
}

void color_parse(std::string format, int index){
	std::string color;
	switch(format[index + 1]){
		case 'r':
			color = RED;
		break;
		case 'g':
			color = GREEN;
		break;
		case 'b':
			color = BLUE;
		break;
		case 'y':
			color = YELLOW;
		break;
		case 'w':
			color = DEFAULT;
		break;
		default:
			printf("[LOG_ERROR] Could not parse the color... exiting!\n");
			exit(-1);
		break;
	}
	parsed_types.push_back(variables(index - 1, color, true));
}

};
