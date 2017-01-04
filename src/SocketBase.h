#ifndef SOCKETBASE_H
#define SOCKETBASE_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdexcept>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include "Message.h"

typedef uint32_t IPAddress;
typedef uint16_t Port;

class SocketBase {
public:
	/*
	* Tylko tworzy gniazdo i konfiguruje je. 
	* By odbierac SIGIO, sigset musi zostac uzupelniony (nie jest tutaj)
	* Rzuca wyjatek przy bledzie.
	*/
	SocketBase();
	/*
	* Zamyka gniazdo.
	*/
	void closeSocket();
	/*
	* Wysyla podana wiadomosc na podany adres.
	*/	
	void sendMessage(Message message, IPAddress destination, Port port);
	/*
	* Pobiera wiadomosc i ustawia wartosc adresu, z ktorego wiadomosc przyszla.
	* Jesli nie ma zadnej wiadomosci, address bedzie rowny 0, a zwrocony Message bedzie mial tag MessageInvalid.
	*/
	Message nextMessage(IPAddress* source_address, Port* source_port);

	struct sockaddr_in 	mSocketAddress;
	int					mSocketFD;

	
protected:
	int htonMessage(Message message, char* buffer);
	Message ntohMessage(char* message);
	const int properMessageSize[8];
};

#endif
