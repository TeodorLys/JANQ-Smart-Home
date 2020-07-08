#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){
	sleep(1);
	execv("/home/pi/udp_janq_command/src/main", 0);
	_exit(0);
}
