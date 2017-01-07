CC = g++
CCFLAGS = -g -Wall -std=c++11

LINK = g++
LINKFLAGS = -g -lrt -std=c++11

OBJS = bin/timer.o bin/server_manager.o bin/SocketPassive.o bin/SocketActive.o bin/SocketBase.o

all: server client

server: $(OBJS) bin/server.o
	$(LINK) $(LINKFLAGS) $(OBJS) bin/server.o -o server

client: $(OBJS) bin/client.o
	$(LINK) $(LINKFLAGS) $(OBJS) bin/client.o -o client

bin/server.o: src/server.cpp
	$(CC) $(CCFLAGS) -c src/server.cpp -o bin/server.o

bin/client.o: src/client.cpp
	$(CC) $(CCFLAGS) -c src/client.cpp -o bin/client.o
	
bin/timer.o: src/timer.cpp
	$(CC) $(CCFLAGS) -c src/timer.cpp -o bin/timer.o
	
bin/server_manager.o: src/server_manager.cpp
	$(CC) $(CCFLAGS) -c src/server_manager.cpp -o bin/server_manager.o

bin/SocketPassive.o: src/SocketPassive.cpp
	$(CC) $(CCFLAGS) -c src/SocketPassive.cpp -o bin/SocketPassive.o

bin/SocketActive.o: src/SocketActive.cpp
	$(CC) $(CCFLAGS) -c src/SocketActive.cpp -o bin/SocketActive.o
	
bin/SocketBase.o: src/SocketBase.cpp
	$(CC) $(CCFLAGS) -c src/SocketBase.cpp -o bin/SocketBase.o
	
clean:
	rm bin/*.o