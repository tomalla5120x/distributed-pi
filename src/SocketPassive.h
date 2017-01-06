#ifndef SOCKETPASSIVE_H
#define SOCKETPASSIVE_H

#include "SocketBase.h"

class SocketPassive: public SocketBase {
public:
	//tylko binduje i zwraca port
	SocketPassive();
	//binduje z zadanym portem (jesli sie da oczywiscie)
	SocketPassive(Port port);
	Port getPort() const { return mPort; }

private:
	Port mPort;

};

#endif
