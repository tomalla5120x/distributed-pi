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

#include <memory>
#include <limits>
#include <iostream>
using namespace std;

#include "easylogging++.h"

#include "connection_main.h"
#include "solution_manager.h"
#include "server_manager.h"

const int32_t MIN_PORT   = 1;
const int32_t MAX_PORT   = (std::numeric_limits<uint16_t>::max());
const int32_t MAX_SIDE   = 9999;
const int32_t MIN_SIDE   = 1;
const int64_t MAX_POINTS = (std::numeric_limits<int64_t>::max());
const int64_t MIN_POINTS = 1;
const int32_t MAX_DIGITS = (std::numeric_limits<int32_t>::max());
const int32_t MIN_DIGITS = 0;

const int SIGNAL_TIMEOUT  = ConnectionMain::getTimerSignal();
const int SIGNAL_HBTIMEOUT = ConnectionMain::getHeartbeatTimerSignal();

bool handleParameter(int32_t& value, char* szValue, string strParam, int32_t nMin, int32_t nMax);
bool handleParameter(int64_t& value, char* szValue, string strParam, int64_t nMin, int64_t nMax);

INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[])
{
    if(argc != 4 && argc != 5)
    {
        cout << "Sposob wywolania programu: " << argv[0] << " <side> <points> <digits> [port]" << endl;
        cout << "  * side   - Precyzuje na ile segmentow nalezy podzielic bok obszaru losowania." << endl;
        cout << "             Obszar zostanie podzielony na side^2 segmentow." << endl;
        cout << "             Liczba calkowita z przedzialu [" << MIN_SIDE << ", " << MAX_SIDE << "]." << endl;
        cout << "  * points - Ile punktow ma zostac wylosowanych w pojedynczym segmencie." << endl;
        cout << "             Liczba calkowita z przedzialu [" << MIN_POINTS << ", " << MAX_POINTS << "]." << endl;
        cout << "  * digits - Z dokladnoscia do ilu cyfr po przecinku nalezy wyprowadzic wynik." << endl;
        cout << "             Liczba calkowita z przedzialu [" << MIN_DIGITS << ", " << MAX_DIGITS << "]." << endl;
        cout << "  * port   - Na jakim porcie serwer ma nasluchiwac." << endl;
        cout << "             Liczba calkowita z przedzialu [" << MIN_PORT << ", " << MAX_PORT << "]." << endl;
        cout << "             Parametr opcjonalny. W razie jego braku port jest przydzielany automatycznie." << endl;
		
		return EXIT_FAILURE;
	}
	
	int32_t nSide, nDigits, nPort=0;
	int64_t nPoints;
	
	if(!handleParameter(nSide, argv[1], "side", MIN_SIDE, MAX_SIDE) ||
		!handleParameter(nPoints, argv[2], "points", MIN_POINTS, MAX_POINTS) ||
		!handleParameter(nDigits, argv[3], "digits", MIN_DIGITS, MAX_DIGITS))
		return EXIT_FAILURE;
	
	if(argc == 5 && !handleParameter(nPort, argv[4], "port", MIN_PORT, MAX_PORT))
		return EXIT_FAILURE;
	
	// =============================================
	
	el::Configurations logConf;
    logConf.setToDefault();
    logConf.set(el::Level::Info, el::ConfigurationType::Format, "%datetime{%Y-%M-%d %H:%m:%s,%g} [ connection %level] %msg");

    el::Loggers::getLogger("connection");
    el::Loggers::reconfigureLogger("connection", logConf);
	
	// maskowanie sygnałów odbieranych w pętli obsługi sygnałów, żeby nie zostały obsłużone domyślnie
	sigset_t set;
	siginfo_t info;
	sigemptyset(&set);
	sigaddset(&set, SIGIO);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGNAL_TIMEOUT);
	sigaddset(&set, SIGNAL_HBTIMEOUT);
	sigprocmask(SIG_BLOCK, &set, NULL);
	
	unique_ptr<SocketPassive> socket;
	
	try
	{
		if(nPort == 0)
			socket.reset(new SocketPassive());
		else
			socket.reset(new SocketPassive(nPort));
	} catch(runtime_error)
	{
		cerr << "Blad tworzenia gniazda." << endl;
		return EXIT_FAILURE;
	}
	
	SolutionManager::configure(nSide, nPoints, nDigits);
	
	ServerManager serverManager(*(socket.get()));
	SolutionManager& solutionManager = SolutionManager::getInstance();
	
	cout << "Ilosc wydzielonych podproblemow: " << solutionManager.initialize() << endl;
	cout << "Serwer nasluchuje na porcie: " << socket->getPort() << endl;
	
	do
	{
		int signal = sigwaitinfo(&set, &info);
		
        if(signal == SIGIO)
        {
            for(;;)
            {
                IPAddress ip;
                Port port;
				
                Message message = socket->nextMessage(&ip, &port);
				
                if(message.getTag() == NoMessage)
                    break;
				
                if(message.getTag() != MessageInvalid)
                    serverManager.handleMessage(message, ip, port);
            }
        } else if(signal == SIGNAL_TIMEOUT)
        {
            serverManager.handleTimeout();
        } else if(signal == SIGNAL_HBTIMEOUT)
        {
            serverManager.handleHeartbeatTimeout();
        } else if(signal == SIGINT)
        {
            serverManager.handleInterrupt();
            cout << "Interrupt." << endl;
            return EXIT_FAILURE;
        }
    } while(!solutionManager.solved());
	
    serverManager.finalize();
    socket->closeSocket();
	
    uint64_t a, b;
    string strResult = solutionManager.getResult(&a, &b);
	
    cout << "Wyznaczone przyblizenie liczby PI: " << endl << endl;
    cout << a << " / " << b << endl << endl;
    cout << strResult << endl;
	
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

bool handleParameter(int64_t& value, char* szValue, string strParam, int64_t nMin, int64_t nMax)
{
    try
    {
        value = stoll(szValue);
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
