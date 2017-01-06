#include "SocketActive.h"

SocketActive::SocketActive(IPAddress address, Port port) : mMainServerPort(port), mMainServerIP(address) {
	//super(); domyslnie
	
	int desc = getSocketDescriptor();
	
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(0);

	if(bind(desc, (const sockaddr*) &sin, sizeof(sockaddr_in)) == -1)
		throw std::runtime_error("Error binding socket!");
}

void SocketActive::send(Message message) {
	sendMessage(message, mMainServerIP, mMainServerPort);
}

Message SocketActive::next(){
	IPAddress srcIP = 0;
	Port srcPort = 0;
	Message toRet = nextMessage(&srcIP, &srcPort);
	
	if(srcIP != mMainServerIP || srcPort != mMainServerPort)
		return Message(MessageInvalid);
	return toRet;
}
