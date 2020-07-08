#pragma once
#include <string>
#include <chrono>
#include <ctime>

class second {
public:
	int t = -1;

	void seconds(int i) { t = i; }
	second() {}
};

class minute : second{
public:
	int t = -1;

	void minutes(int i) { t = i; }
	minute() {}
};

class hour : minute{
public:
	int t = -1;

	void hours(int i) { t = i; }
	hour() {}
};

class day : hour {
public:
	int t = -1;

	void days(int i) { t = i; }
	day() {}
};

class timer {
private:
	static second _s;
	static minute _m;
	static hour _h;
	static day _d;
	static timer _t;

	static int __s;
	static int __m;
	static int __h;
	static int __d;

public:
	static timer seconds(int i) { _s.seconds(i); return _t;}
	static timer minutes(int i) { _m.minutes(i); return _t;}
	static timer hours(int i) { _h.hours(i); return _t; }
	static timer days(int i) { _d.days(i); return _t;}

	static bool approx_ge() {

		if (_d.t >= __d && _d.t != -1)
			return false;

		if (_h.t >= __h || _h.t == -1)
			if (_m.t >= __m || _m.t == -1)
				if (_s.t >= __s || _s.t == -1)
					return true;
			else
				return false;
		else
			return false;

		return false;
	}

	static bool approx_le() {

		if (_d.t >= __d && _d.t != -1)
			return false;

		if (__h <= _h.t || _h.t == -1)
			if (__m <= _m.t || _m.t == -1)
				if (__s <= _s.t || _s.t == -1)
					return true;
				else
					return false;
			else
				return false;

		return false;
	}

	timer() { 
		_t = *this; 
		auto end = std::chrono::system_clock::now();
		std::time_t t = std::chrono::system_clock::to_time_t(end);
		std::string temp = std::ctime(&t);

		std::string buff = temp;
		buff.erase(0, buff.find(":") - 2);
		buff.erase(buff.find(":"));

		__h = stoi(buff);

		buff = temp;
		buff.erase(0, buff.find(":") + 1);
		buff.erase(buff.find(":"));

		__m = stoi(buff);

		buff = temp;
		buff.erase(0, buff.find(":") + 1);
		buff.erase(0, buff.find(":") + 1);
		buff.erase(2, buff.length());

		__s = stoi(buff);

		if (temp.find("Mon") != std::string::npos) {
			__d = 1;
		}
		else if (temp.find("Tue") != std::string::npos) {
			__d = 2;
		}
		else if (temp.find("Wed") != std::string::npos) {
			__d = 3;
		}
		else if (temp.find("Thu") != std::string::npos) {
			__d = 4;
		}
		else if (temp.find("Fri") != std::string::npos) {
			__d = 5;
		}
		else if (temp.find("Sat") != std::string::npos) {
			__d = 6;
		}
		else if (temp.find("Sun") != std::string::npos) {
			__d = 7;
		}

		printf("D: %i, H: %i, M: %i, S: %i\n", __d, __h, __m, __s);

	}
	void print() {
		printf("D: %i, H: %i, M: %i, S: %i\n", _d.t, _h.t, _m.t, _s.t);
	}
};

second timer::_s;
minute timer::_m;
hour timer::_h;
day timer::_d;
int timer::__s;
int timer::__m;
int timer::__h;
int timer::__d;
timer timer::_t;