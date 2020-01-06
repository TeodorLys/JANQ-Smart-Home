#pragma once
#include <SFML/Network.hpp>

class udp_container {
private:

public:
	static sf::UdpSocket socket;
	udp_container() {}

	static sf::UdpSocket& get_socket() {
		return socket;
	}
};

sf::UdpSocket udp_container::socket;
