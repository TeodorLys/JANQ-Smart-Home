#pragma once
#include <SFML/Network.hpp>

class udp_container {
private:

public:
	static sf::UdpSocket socket;
	static sf::UdpSocket host_com;
  static sf::UdpSocket forwarding_com;
	udp_container() {
    
  }

  static void bind_sockets(){
    if(host_com.bind(4133) != sf::Socket::Done){
      printf("Could not bind to host port...\n");
      exit(0);
    }

    if(socket.bind(4123) != sf::Socket::Done){
      printf("Could not bind to port...\n");
      exit(0);
    }

    if(forwarding_com.bind(6123) != sf::Socket::Done){
      printf("Could not bind to port...\n");
      exit(0);
    }
  }

	static sf::UdpSocket& get_host_com() {
		return host_com;
	}

	static sf::UdpSocket& get_socket() {
		return socket;
	}

	static sf::UdpSocket& get_forwarding_com() {
		return forwarding_com;
	}

	static std::string get_ip(){
		return sf::IpAddress::getLocalAddress().toString();
	}
};

sf::UdpSocket udp_container::socket;
sf::UdpSocket udp_container::host_com;
sf::UdpSocket udp_container::forwarding_com;
