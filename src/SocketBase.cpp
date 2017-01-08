#include <arpa/inet.h>

#include "SocketBase.h"

//podpatrzone z SO
#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))


SocketBase::SocketBase() : properMessageSize{1, 1, 1, 5, 5, 5, 17, 21} {
	mSocketFD = socket(AF_INET, SOCK_DGRAM, 0);
	if(mSocketFD == -1)
		throw std::runtime_error("Error creating socket!");
	pid_t pgrp = getpid();
	if(ioctl(mSocketFD, SIOCSPGRP, &pgrp) < 0)
		throw std::runtime_error("Error setting the process!");
	int on = 1;
	if(ioctl(mSocketFD, FIOASYNC, &on) < 0)
		throw std::runtime_error("Error enabling async signals!");	
}

SocketBase::~SocketBase()
{
	closeSocket();
}

void SocketBase::closeSocket() {
	close(mSocketFD);
}

void SocketBase::sendMessage(Message message, IPAddress destination, Port port) {
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(destination);
	sin.sin_port = htons(port); //port powinien byc znany

	int msgSize = properMessageSize[message.getTag()];
	char buffer[24];
	htonMessage(message, buffer);
	sendto(mSocketFD, (void*) buffer, msgSize, 0, (const sockaddr*) &sin, sizeof(struct sockaddr_in));
	
}

Message SocketBase::nextMessage(IPAddress* source_address, Port* source_port) {

	struct sockaddr_in source;
	*source_address = 0;
	*source_port = 0;
	char buffer[24];

	//dzieki fladze MSG_DONTWAIT, jesli na gniezdzie nie ma nic ciekawego,
	//recv zwraca -1 i ustawia errno na odpowiednia wartosc
	//w przeciwnym wypadku, zwraca dlugosc otrzymanego datagramu
	socklen_t socksize = sizeof(sockaddr_in);
	int result = recvfrom(mSocketFD, (void*) buffer, sizeof(Message), MSG_DONTWAIT, (sockaddr*) &source, &socksize);
	if(result < 0)
	{
		int errval = errno;
		if(errval == EWOULDBLOCK || errval == EAGAIN) //brak wiadomosci - zwroc MessageInvalid
			return Message(NoMessage);
		else
			throw std::runtime_error("Error receiving message, other than no value! "+errval);
	}
	//przekonwertuj wiadomosc
	Message toReturn = ntohMessage(buffer);

	if(toReturn.getTag() > 7 || properMessageSize[toReturn.getTag()] != result) //zdecydowanie zly rozmiar datagramu, ignorujemy
		return Message(MessageInvalid);

	//poprawna wiadomosc (pod wzgledem dlugosci):


	*source_address = ntohl(source.sin_addr.s_addr);
	*source_port = ntohs(source.sin_port);
	return toReturn;

}

Message SocketBase::ntohMessage(char* message) {
	MessageType type = (MessageType) message[0]; //ten nie jest konwertowany, wiec mozna bezposrednio pobrac wartosc typu

	Message toReturn(MessageInvalid);

	switch(type)
	{
		case MessageHeartbeat: case MessageHeartbeatACK: case MessageInterrupt:
			toReturn = Message(type);
			break;
		case MessageHello: case MessageACK: case MessageClose:
			toReturn = Message(type, ntohl( *(uint32_t*)(message+1)) ); //zastanawiam sie, czy mozna to zrobic ladniej
			break;
		case MessageResult:
			toReturn = Message(type, 
						ntohl( *(uint32_t*) (message+1)), //sequence
						ntohl( *(uint32_t*) (message+13)), //segmentID
						ntohll(*(uint64_t*) (message+5)) //points_hit
					  );
			break; //pewnie nie
		case MessageWork:
			toReturn = Message(type, 
						ntohl( *(uint32_t*) (message+1)), //sequence
						ntohl( *(uint32_t*) (message+13)),//segmentID
						ntohll(*(uint64_t*) (message+5)), //points
						ntohl( *(uint32_t*) (message+17)) //side
					  );
			break; //wazne, ze dziala
		default:
			toReturn = Message(MessageInvalid); //jedynie konstruktor message mozna by bylo zmienic zeby sie kolejnosc zgadzala
	}	
	return toReturn;
}

int SocketBase::htonMessage(Message message, char* buffer) {
	int size = 0;
	
	//passthrough
	switch(message.getTag())
	{
		case MessageWork:
			size += 4;
			*(uint32_t*)(buffer + 17) = htonl(message.getSide());
		case MessageResult:
			size += 12;
			*(uint32_t*)(buffer + 13) = htonl(message.getSegmentID());
			*(uint64_t*)(buffer + 5) = htonll(message.getPoints()); //points i points_hit to to samo
		case MessageHello: case MessageACK: case MessageClose:
			size += 4;
			*(uint32_t*)(buffer + 1) = htonl(message.getSequence());
		case MessageHeartbeat: case MessageHeartbeatACK: case MessageInterrupt:
		default:
			size += 1; 
			buffer[0] = message.getTag();
	}	
	
	return size;
}

int SocketBase::getSocketDescriptor() const {
	return mSocketFD;
}

/*
* Konwertuje adres IP (host-order) na czytelny napis w standardowym dot-notation.
* Napis jest alokowany statycznie - każde kolejne wywołanie nadpisuje bufor (patrz: inet_ntoa)
*/
char* SocketBase::iptostr(IPAddress ip)
{
	in_addr a;
	a.s_addr = htonl(ip);
	return inet_ntoa(a);
}

/*
* Konwertuje napis w standardowym dot-notation na adres IP (host-order).
* Zwraca true jeżeli konwersja się powiodła, false w przeciwnym razie.
*/
bool SocketBase::strtoip(char* str, IPAddress* ip)
{
	in_addr a;
	if(!inet_aton(str, &a))
		return false;
	*ip = (IPAddress)(ntohl(a.s_addr));
	return true;
}