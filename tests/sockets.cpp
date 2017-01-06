//g++ -std=c++11 SocketBase.cpp SocketPassive.cpp SocketActive.cpp "../tests/sockets.cpp" -o "../tests/sockets"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>

#include <iostream>

#include "../src/SocketActive.h"
#include "../src/SocketPassive.h"

using namespace std;

int main()
{
	SocketPassive socketMain;
	
	Port mainPort = socketMain.getPort();
	
	// maskowanie SIGIO
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGIO);
	siginfo_t info;
	
	// zablokowanie tych sygnałów, żeby nie zostały obsłużone domyślnie (pomiędzy wywołaniami sigwaitinfo)
	sigprocmask(SIG_BLOCK, &set, NULL);
	
	if(fork() == 0)
	{
		// child - worker
		socketMain.closeSocket();
		
		IPAddress address;
		
		if(!SocketBase::strtoip("127.0.0.1", &address))
		{
			cout << "Wrong ip address. " << endl;
			return 1;
		}
		
		SocketActive socketWorker(address, mainPort);
		
		cout << "Connected. Sending test packets. " << endl;
		
		// --------------------------------------
		
		Message msg;
		int signal_number;
		
		sleep(2);
		cout << "Sending Heartbeat ... " << endl;
		socketWorker.send(Message(MessageHeartbeat));
		
		sigwaitinfo(&set, &info);
		while((msg = socketWorker.next()).getTag() != MessageInvalid)
			if(msg.getTag() == MessageACK)
				cout << "Received ACK." << endl;
			else
				cout << "Received unknown message." << endl;;
		
		sleep(2);
		cout << "Sending HeartbeatACK ... " << endl;
		socketWorker.send(Message(MessageHeartbeatACK));
		
		sigwaitinfo(&set, &info);
		while((msg = socketWorker.next()).getTag() != MessageInvalid)
			if(msg.getTag() == MessageACK)
				cout << "Received ACK." << endl;
			else
				cout << "Received unknown message." << endl;
		
		sleep(2);
		cout << "Sending Interrupt ... " << endl;
		socketWorker.send(Message(MessageInterrupt));
		
		sigwaitinfo(&set, &info);
		while((msg = socketWorker.next()).getTag() != MessageInvalid)
			if(msg.getTag() == MessageACK)
				cout << "Received ACK." << endl;
			else
				cout << "Received unknown message." << endl;
		
		sleep(2);
		cout << "Sending Hello ... " << endl;
		socketWorker.send(Message(MessageHello, 123456789));
		
		sigwaitinfo(&set, &info);
		while((msg = socketWorker.next()).getTag() != MessageInvalid)
			if(msg.getTag() == MessageACK)
				cout << "Received ACK." << endl;
			else
				cout << "Received unknown message." << endl;
		
		sleep(2);
		cout << "Sending ACK ... " << endl;
		socketWorker.send(Message(MessageACK, 987654321));
		
		sigwaitinfo(&set, &info);
		while((msg = socketWorker.next()).getTag() != MessageInvalid)
			if(msg.getTag() == MessageACK)
				cout << "Received ACK." << endl;
			else
				cout << "Received unknown message." << endl;
	
		sleep(2);
		cout << "Sending Close ... " << endl;
		socketWorker.send(Message(MessageClose, 111666999));
		
		sigwaitinfo(&set, &info);
		while((msg = socketWorker.next()).getTag() != MessageInvalid)
			if(msg.getTag() == MessageACK)
				cout << "Received ACK." << endl;
			else
				cout << "Received unknown message." << endl;
		
		sleep(2);
		cout << "Sending Result ... " << endl;
		socketWorker.send(Message(MessageResult, 999666111, 123123123, 10908070605040302010U));
		
		sigwaitinfo(&set, &info);
		while((msg = socketWorker.next()).getTag() != MessageInvalid)
			if(msg.getTag() == MessageACK)
				cout << "Received ACK." << endl;
			else
				cout << "Received unknown message." << endl;
		
		sleep(2);
		cout << "Sending Work ... " << endl;
		socketWorker.send(Message(MessageWork, 999666111, 123123123, 12345678999999999990U, 666777888));
		
		sigwaitinfo(&set, &info);
		while((msg = socketWorker.next()).getTag() != MessageInvalid)
			if(msg.getTag() == MessageACK)
				cout << "Received ACK." << endl;
			else
				cout << "Received unknown message." << endl;
		
	} else
	{
		// original process - main server
		for(;;)
		{
			IPAddress addr;
			Port port;
			
			int signal_number = sigwaitinfo(&set, &info);
			
			if(signal_number != SIGIO)
				continue;
			
			for(;;)
			{
				Message msg = socketMain.nextMessage(&addr, &port);
				
				if(msg.getTag() == MessageInvalid)
					break;
				
				cout << endl << "Received message: " << endl;
				cout << " tag: " << msg.getTag() << endl;
				cout << " sequence: " << msg.getSequence() << endl;
				cout << " points (aka pointshit): " << msg.getPoints() << endl;
				cout << " segment id: " << msg.getSegmentID() << endl;
				cout << " side: " << msg.getSide() << endl;
				cout << endl;
				
				socketMain.sendMessage(Message(MessageACK), addr, port);
			}
		}
	}
	
	return 0;
}