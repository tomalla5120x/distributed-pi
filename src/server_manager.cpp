#include "server_manager.h"
#include "connection_main.h"

#define POOL_SIZE 10

void ServerManager::handleMessage(Message& message, const IPAddress ip, const Port port)
{
	// wywoływane z poziomu pętli obsługi sygnałów, kiedy nadejdzie jakaś wiadomość.

	// Metody handleMessage oraz handleTimeout delegowałyby obsługę do konkretnego obiektu Connection,
	// a jeżeli takiego nie ma, to same by go obsługiwały
	// (np. jak Hello, to tworzy nowy Connection i jemu przekazują wiadomość Hello do obsługi,
	// w przeciwnym razie ignorują go).
	
	ConnectionMain* pConn = m_pool.getConnection(ip, port);
	
	if(pConn == nullptr)
	{
		if(m_pool.isFull() || message.getTag() != MessageHello)
			return;
		
		//TODO: stwórz nowy obiekt ConnectionMain
		//pConn = new ConnectionMain(...);
		
		m_pool.addConnection(pConn);
		
		//TODO: przekaż wiadomość MessageHello obiektowi ConnectionMain
		//pConn->handleMessage(message);
	} else
	{
		//TODO: przekaż wiadomość MessageHello obiektowi ConnectionMain
		//pConn->handleMessage(message);
	}
}

void ServerManager::handleTimeout()
{
	// wywoływane z poziomiu pętli obsługi sygnałów, kiedy zostanie odebrany sygnał 
	// wygenerowany przez któryś z timerów Timeout. Wywołanie zostaje oddelegowywane 
	// do odpowiedniego ConnectionMain. Jeżeli zwróci false, trzeba odpowiednio zareagować
	// (np. usunąć ConnectionMain z puli, przypisać komuś innemu podproblem itd.).
	
	m_pool.deleteConnectionIf([](ConnectionMain* pConn) -> bool {
		//TODO:
		//if(pConn->isTimeoutExpired())
		//	return !pConn->handleTimeout();
		
		return false;
	});
}

void ServerManager::handleHeartbeatTimeout()
{
	// wywoływane z poziomiu pętli obsługi sygnałów, kiedy zostanie odebrany sygnał
	// wygenerowany przez któryś z timerów Heartbeat Timeout. Ustalamy, timery których
	// obiektów ConnectionMain wygasły i coś z nimi robimy (kasujemy ConnectionMain z puli, sprzątamy po nim itd.).
	
	m_pool.deleteConnectionIf([](ConnectionMain* pConn) -> bool {
		//TODO:
		return false /*pConn->isHeartbeatTimeoutExpired()*/;
	});
}

void ServerManager::handleInterrupt()
{
	// wywoływane z poziomiu pętli obsługi sygnałów, kiedy odebrany zostanie sygnał SIGINT.
	// Wysyła wszystkim serwerom roboczym wiadomość Interrupt.
	
	m_pool.deleteConnectionIf([](ConnectionMain* pConn) -> bool {
		//TODO:
		//pConn->sendMessage(Message(MessageInterrupt));
		
		return true;
	});
}

void ServerManager::finalize()
{
	// wysyła wszystkim wiadomość Close - wszystkie wyniki cząstkowe
	// zostały już dostarczone, serwery robocze nie są potrzebne.

	m_pool.deleteConnectionIf([](ConnectionMain* pConn) -> bool {
		//TODO:
		//pConn->sendMessage(Message(MessageClose));
		
		return false;
	});
}

// =============================================================================

// zwraca połączenie identyfikowane przez adres IP i port lub przez ServerID
// zwraca nullptr jeżeli takiego połączenia nie ma
ConnectionMain* ConnectionPool::getConnection(const IPAddress ip, const Port port)
{
	return getConnection(SID{ip, port});
}

ConnectionMain* ConnectionPool::getConnection(const SID sid)
{
	if(m_map.count(sid) == 0)
		return nullptr;
	return m_map[sid];
}

bool ConnectionPool::isFull() const
{
	return m_map.size() == POOL_SIZE;
}

void ConnectionPool::addConnection(ConnectionMain* pConn)
{
	//TODO: wydobycie IPAddress oraz Port z pConn
	IPAddress ip = 666;
	Port port = 666;
	
	//m_map[SID{ip, port}] = std::unique_ptr<ConnectionMain>(pConn);
	m_map[SID{ip, port}] = pConn;
}

void ConnectionPool::deleteConnectionIf(std::function<bool(ConnectionMain*)> predicate)
{
	ConnectionMap::iterator it = m_map.begin();
	while(it != m_map.end())
	{	
		if(predicate(it->second))
		{
			delete it->second;
			it = m_map.erase(it);
		}
		else
			++it;
		
		/*
		std::unique_ptr<ConnectionMain>* pP = &(it->second);
		
		if(predicate(pP->get()))
			it = m_map.erase(it);
		else
			++it;
		*/
	}
}

ConnectionPool::~ConnectionPool()
{
	for(ConnectionMap::iterator it = m_map.begin(); it != m_map.end(); ++it)
		delete it->second;
}