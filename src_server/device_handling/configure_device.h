#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/wireless.h>
#include <errno.h>
#include <thread>
#include "../log_printing.h"
#include "../net_request.h"

#define IW_INTERFACE "wlan0"
extern int errno;
class configure_device {
private:
struct iwreq wreq;

public:

void search_for_new_device(std::string _ssid, std::string _pass, std::string _name){
  net_request req;
  log_printing(HIGH, "%cy[SYSMESSAGE]%cw Trying to connect to JANQ_ap.\n");
  std::string def_id = get_current_ssid();
  try_janq_ap();
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  std::string _id = get_current_ssid();
  if(!validate_ssid("JANQ_ap")){
      log_printing(HIGH, "Sending FAILED to hosts, %s %crFAILED%cw\n", def_id.c_str());
      reset_wifi();
      if(validate_ssid(def_id)){
        reconnect_wpa();
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        req.rebind_host_socket();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        req.send_to_hosts("RND_FAILED", true);
      }
      return;
  }
  log_printing(HIGH, "%cgOK%cw\n");
  log_printing(HIGH, "%cy[SYSMESSAGE]%cw Sending credentials.\n");
  int count = 0;
  bool nope = true;
  if(!req._remote_net_request("192.168.1.1/actionpage.php?ssid=" + _ssid + "&pass=" + _pass + "&name=" + _name, false)){
    while(count < 20){
      std::this_thread::sleep_for(std::chrono::seconds(10));
      if(req._remote_net_request("192.168.1.1/actionpage.php?ssid=" + _ssid + "&pass=" + _pass + "&name=" + _name, false)){
        nope = false;
        break;
      }
      count++;
      if(count % 5 == 0)
        reconnect_wpa();
      log_printing(HIGH, "Attempt %i\n", count);
    }

  }
  log_printing(HIGH, "[SYSMESSAGE] Reconnecting to default ssid\n");
  reset_wifi();
  if(validate_ssid(def_id)){
    if(nope){
      reconnect_wpa();
      std::this_thread::sleep_for(std::chrono::milliseconds(10000));
      req.rebind_host_socket();
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      req.send_to_hosts("RND_FAILED", true);
      log_printing(HIGH, "Sending FAILED to hosts %crFAILED%cw\n");
    }else {
      reconnect_wpa();
      std::this_thread::sleep_for(std::chrono::milliseconds(10000));
      req.rebind_host_socket();
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      req.send_to_hosts("RND_OK", true);
      log_printing(HIGH, "Sending OK to hosts %cgOK%cw\n");
    }
  }
}

private:

  bool validate_ssid(std::string _s){
    int count = 0;
    std::string _id = get_current_ssid();
    while(count < 5){
      log_printing(HIGH, "Attempt %i\n", count + 1);
      _id = get_current_ssid();
      count++;
      if(_id != "")
        break;
      std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
    if(_id != _s){
      return false;
    }else {
      return true;
    }
  }

  std::string get_current_ssid(){
  int sockfd;
    char * id;
   id = new char[IW_ESSID_MAX_SIZE+1];

    struct iwreq wreq;
    memset(&wreq, 0, sizeof(struct iwreq));
    wreq.u.essid.length = IW_ESSID_MAX_SIZE+1;

    sprintf(wreq.ifr_name, IW_INTERFACE);

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        fprintf(stderr, "Cannot open socket \n");
        fprintf(stderr, "errno = %d \n", errno);
        fprintf(stderr, "Error description is : %s\n",strerror(errno));
        return "";
    }
    //printf("\nSocket opened successfully \n");

    wreq.u.essid.pointer = id;
    if (ioctl(sockfd,SIOCGIWESSID, &wreq) == -1) {
        fprintf(stderr, "Get ESSID ioctl failed \n");
        fprintf(stderr, "errno = %d \n", errno);
        fprintf(stderr, "Error description : %s\n",strerror(errno));
        return "";
    }

    //printf("IOCTL Successfull\n");

    return (char *)wreq.u.essid.pointer;
  }

  void try_janq_ap(){
    system("sudo wpa_cli -i wlan0 select_network 1");
    system("sudo wpa_cli -i wlan0 reconnect > /dev/null");
  }
  void reconnect_wpa(){
    system("sudo wpa_cli -i wlan0 reconnect > /dev/null");
  }

  void reset_wifi(){
    system("sudo wpa_cli -i wlan0 select_network 0");
    log_printing(HIGH, "%cy[SYSMESSAGE]%cw Going back to default Wifi!\n");
  }

};