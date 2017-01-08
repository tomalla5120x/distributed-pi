#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <memory>
#include <map>

#include "SocketActive.h"
#include "SocketPassive.h"

typedef struct {
	IPAddress ip;
	Port port;
} SID;

struct _SIDCompare
{
	bool operator() (const SID& lhs, const SID& rhs) const
	{
		if(lhs.ip < rhs.ip)
			return true;
		if(lhs.ip > rhs.ip)
			return false;
		return (lhs.port < rhs.port);
	}
};

class ConnectionMain;
class ConnectionPool
{
public:
	~ConnectionPool();
	
	// zwraca połączenie identyfikowane przez adres IP i port lub przez ServerID
	// zwraca nullptr jeżeli takiego połączenia nie ma
	ConnectionMain* getConnection(const IPAddress ip, const Port port);
	ConnectionMain* getConnection(const SID);
	
	// sprawdza, czy pula serwerów jest już pełna
	bool isFull() const;
	
	// dodaje do puli wskazany obiekt ConnectionMain
	void addConnection(ConnectionMain*);
	
	// iteruje po wszystkich połączeniach z puli
	// dodatkowo jeżeli predykat zwróci true, połączenie zostaje usunięte z puli
	void deleteConnectionIf(std::function<bool(ConnectionMain*)> predicate);
	
private:
	typedef std::map<SID, ConnectionMain*, _SIDCompare> ConnectionMap;

	ConnectionMap m_map;
};

class ServerManager
{	
public:
	void handleMessage(Message& message, const IPAddress ip, const Port port);
	void handleTimeout();
	void handleHeartbeatTimeout();
	void handleInterrupt();
	void finalize();
	
private:
	ConnectionPool m_pool;
};

#endif