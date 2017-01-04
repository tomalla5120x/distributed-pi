#include "SocketPassive.h"

SocketPassive::SocketPassive() {
	//super(); domyslnie
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(0);

	if(bind(mSocketFD, (const sockaddr*) &sin, sizeof(sockaddr_in)) == -1)
		throw std::runtime_error("Error binding socket!");
	
	mPort = ntohs(sin.sin_port);
}

SocketPassive::SocketPassive(Port port) {
	//super(); domyslnie
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(port);

	if(bind(mSocketFD, (const sockaddr*) &sin, sizeof(sockaddr_in)) == -1)
		throw std::runtime_error("Error binding socket!");
	
	mPort = ntohs(sin.sin_port);
}
