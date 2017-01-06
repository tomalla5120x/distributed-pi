#include "SocketPassive.h"

SocketPassive::SocketPassive() {
	//super(); domyslnie
	
	int desc = getSocketDescriptor();
	
	sockaddr_in sin;
	socklen_t length = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(0);

	if(bind(desc, (const sockaddr*) &sin, sizeof(sockaddr_in)) == -1)
		throw std::runtime_error("Error binding socket!");
	
	if(getsockname(desc, (struct sockaddr *) &sin, &length) == -1) {
		throw std::runtime_error("Error getting socket name!");
	}
	
	mPort = ntohs(sin.sin_port);
}

SocketPassive::SocketPassive(Port port) {
	//super(); domyslnie
	
	int desc = getSocketDescriptor();
	
	sockaddr_in sin;
	socklen_t length = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(port);

	if(bind(desc, (const sockaddr*) &sin, sizeof(sockaddr_in)) == -1)
		throw std::runtime_error("Error binding socket!");
	
	if(getsockname(desc, (struct sockaddr *) &sin, &length) == -1) {
		throw std::runtime_error("Error getting socket name!");
	}
	
	mPort = ntohs(sin.sin_port);
}
