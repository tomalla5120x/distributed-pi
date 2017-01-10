//g++ -std=c++11 SocketBase.cpp SocketPassive.cpp SocketActive.cpp "../tests/wrong_order.cpp" -o "../tests/wrong_order"

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
	

		// child - worker
		socketMain.closeSocket();
		
		IPAddress address;
		
		if(!SocketBase::strtoip("192.168.2.250", &address))
		{
			cout << "Wrong ip address. " << endl;
			return 1;
		}
		
		SocketActive socketWorker(address, 50001);
		
		cout << "Connected. Sending test packets. " << endl;
		
		// --------------------------------------
		
		Message msg;
		int signal_number;
		uint64_t pts;
		uint32_t id;
		sleep(2);
		
		cout << "Sending Hello ... " << endl;
		socketWorker.send(Message(MessageHello, 1));
		sleep(1);
		
		sigwaitinfo(&set, &info);
		while((msg = socketWorker.next()).getTag() == NoMessage);
			if(msg.getTag() == MessageWork)
			{
				pts = msg.getPoints();
				id = msg.getSegmentID();
				cout << "Received Work. " << msg.getSequence() << " " << pts << " " << id << endl;
			}
			else
				cout << "Received unknown message." << msg.getTag()<< endl;

		cout << "Sending Result ... " << endl;
		socketWorker.send(Message(MessageResult, 4, id, pts-15));

		cout << "Sending ACK ... " << endl;
		socketWorker.send(Message(MessageACK, 3));


	return 0;
}
