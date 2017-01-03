#include "SocketBase.h"


SocketBase::SocketBase() {
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

void SocketBase::close() {
	close(mSocketFD);
}

void SocketBase::sendMessage(Message message, IPAddress destination, Port port) {
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(destination);
	sin.sin_port = htons(port); //port powinien byc znany

	sendto(mSocketFD, (void*) &message, sizeof(Message), 0, &sin, sizeof(struct sockaddr_in);
	
}

Message SocketBase::nextMessage(IPAddress* source_address, Port* source_port) {
	Message toReturn(MessageInvalid);
	struct sockaddr_in source;
	*source_address = 0;
	*source_port = 0;

	//dzieki fladze MSG_DONTWAIT, jesli na gniezdzie nie ma nic ciekawego,
	//recv zwraca -1 i ustawia errno na odpowiednia wartosc
	//w przeciwnym wypadku, zwraca dlugosc otrzymanego datagramu
	int result = recvfrom(mSocketFD, &toReturn, sizeof(Message), MSG_DONTWAIT, &source, sizeof(struct sockaddr_in));
	if(result < 0)
	{
		int errval = errno;
		if(errval == EWOULDBLOCK || errval == EAGAIN) //brak wiadomosci - zwroc MessageInvalid
			return toReturn;
		else
			throw std::runtime_error("Error receiving message, other than no value! "+errval);
	}
	if(result != sizeof(Message)) //zly rozmiar datagramu, ignorujemy
		return Message(MessageInvalid);

	//poprawna wiadomosc (pod wzgledem dlugosci):
	*source_address = ntohl(source.sin_addr.s_addr);
	*source_port = ntosl(source.sin_port);
	return toReturn;

}
