CC = g++
CCFLAGS = -g -Wall -Wextra -O3 -std=c++11

LINK = g++
LINKFLAGS = -g -Wall -Wextra -O3 -lrt -std=c++11

OBJS = bin/timer.o bin/SocketPassive.o bin/SocketActive.o bin/SocketBase.o bin/subproblem.o bin/segment.o bin/solution_manager.o bin/worker_thread.o bin/server_manager.o bin/connection_main.o bin/connection_worker.o

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





bin/worker_thread.o: src/worker_thread.cpp
	$(CC) $(CCFLAGS) -c src/worker_thread.cpp -o bin/worker_thread.o

bin/subproblem.o: src/subproblem.cpp
	$(CC) $(CCFLAGS) -c src/subproblem.cpp -o bin/subproblem.o

bin/solution_manager.o: src/solution_manager.cpp
	$(CC) $(CCFLAGS) -c src/solution_manager.cpp -o bin/solution_manager.o

bin/segment.o: src/segment.cpp
	$(CC) $(CCFLAGS) -c src/segment.cpp -o bin/segment.o






bin/connection_worker.o: src/connection_worker.cpp
	$(CC) $(CCFLAGS) -c src/connection_worker.cpp -o bin/connection_worker.o
	
bin/connection_main.o: src/connection_main.cpp
	$(CC) $(CCFLAGS) -c src/connection_main.cpp -o bin/connection_main.o
	
	
	
	

bin/SocketPassive.o: src/SocketPassive.cpp
	$(CC) $(CCFLAGS) -c src/SocketPassive.cpp -o bin/SocketPassive.o

bin/SocketActive.o: src/SocketActive.cpp
	$(CC) $(CCFLAGS) -c src/SocketActive.cpp -o bin/SocketActive.o
	
bin/SocketBase.o: src/SocketBase.cpp
	$(CC) $(CCFLAGS) -c src/SocketBase.cpp -o bin/SocketBase.o
	
clean:
	rm bin/*.o