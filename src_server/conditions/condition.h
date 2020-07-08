#pragma once
#include <chrono>
#include <ctime>
#include "timer.h"

class condition {

private:
	bool ret = false;
public:

	condition(timer _t, bool greater = true) {
		if(greater)
			ret = _t.approx_ge();
		else 
			ret = _t.approx_le();
	}

	operator bool() const {
		return true;
	}

};