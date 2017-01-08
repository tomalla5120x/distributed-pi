#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include <cstring>
#include <iostream>

using namespace std;

#include "worker_thread.h"
#include "connection_worker.h"
#include "SocketActive.h"

const int SIGNAL_TIMER  = ConnectionWorker::getTimerSignal();
const int SIGNAL_THREAD = WorkerThread::getThreadSignal();
const int SIGNAL_HBTIMEOUT = -1; //TODO:
	
	
const int32_t MIN_PORT = 1;
const int32_t MAX_PORT = (std::numeric_limits<uint16_t>::max());

bool handleParameter(int32_t& value, char* szValue, string strParam, int32_t nMin, int32_t nMax);

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		cout << "Sposob wywolania programu: " << argv[0] << " <ip-address> <port>" << endl;
		cout << "  * ip-address - Adres IP serwera glownego w standardowym dot-notation." << endl;
		cout << "  * port       - Port na ktorym serwer glowny nasluchuje." << endl;
		cout << "                 Liczba calkowita z przedzialu [" << MIN_PORT << ", " << MAX_PORT << "]." << endl;
		
		return EXIT_FAILURE;
	}
	
	IPAddress ip;
	
	if(!SocketBase::strtoip(argv[1], &ip))
	{
		cerr << "Podany adres IP jest niepoprawny." << endl;
		return EXIT_FAILURE;
	}
	
	int nPort;
	if(!handleParameter(nPort, argv[2], "port", MIN_PORT, MAX_PORT))
		return EXIT_FAILURE;
	
	Port port = (Port)nPort;
	
	// ========================================
	
	// maskowanie sygnałów odbieranych w pętli obsługi sygnałów, żeby nie zostały obsłużone domyślnie
	sigset_t set;
	siginfo_t info;
	sigemptyset(&set);
	sigaddset(&set, SIGIO);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGNAL_TIMER);
	sigaddset(&set, SIGNAL_THREAD);
	sigprocmask(SIG_BLOCK, &set, NULL);
	
	SocketActive socket(ip, port);
	ConnectionWorker connection(socket);
	
	for(;;)
	{
		int signal = sigwaitinfo(&set, &info);
		
		if(signal == SIGIO)
		{
			for(;;)
			{
				Message message = socket.next();
				
				if(message.getTag() == NoMessage)
					break;
				
				if(message.getTag() != MessageInvalid)
					if(!connection.handleMessage(message))
					{
						cout << "Zakonczenie pracy. " << endl;
						return EXIT_SUCCESS;
					}
			}
		} else if(signal == SIGNAL_TIMER)
		{
			connection.handleTimeout();
		} else if(signal == SIGNAL_THREAD)
		{
			connection.sendResult();
		} else if(signal == SIGNAL_HBTIMEOUT || signal == SIGINT)
		{
			connection.sendInterrupt();
			cout << "Interrupt." << endl;
			return EXIT_FAILURE;
		}
	}
	
	return EXIT_SUCCESS;
}

bool handleParameter(int32_t& value, char* szValue, string strParam, int32_t nMin, int32_t nMax)
{
	try
	{
		value = stoi(szValue);
		if(value >= nMin && value <= nMax)
			return true;
	} catch(invalid_argument)
	{
		cerr << "Parametr <" << strParam << "> jest niepoprawny." << endl;
		return false;
	} catch(out_of_range)
	{}
	
	cerr << "Parametr <" << strParam << "> musi byc z zakresu [" << nMin << ", " << nMax << "]." << endl;
	
	return false;
}