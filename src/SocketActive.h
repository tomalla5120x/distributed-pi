#ifndef SOCKETACTIVE_H
#define SOCKETACTIVE_H

#include "SocketBase.h"

class SocketActive: public SocketBase {
public:
	//binduje i zapisuje adres i port na pozniej
	SocketActive(IPAddress address, Port port);
	//wrapper
	void send(Message message);
	//wrapper z porownaniem
	Message next();

private:
	Port mMainServerPort;
	IPAddress mMainServerIP;

};

#endif
