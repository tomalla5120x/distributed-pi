CC = g++
CCFLAGS = -g -Wall -Wextra -O3 -std=c++11 -pthread 

LINK = g++
LINKFLAGS = -g -Wall -Wextra -O3  -std=c++11 -pthread -lrt

OBJS = bin/timer.o bin/SocketPassive.o bin/SocketActive.o bin/SocketBase.o bin/subproblem.o bin/segment.o bin/solution_manager.o bin/worker_thread.o bin/server_manager.o bin/connection_main.o bin/connection_worker.o

all: server_main server_worker

server_main: $(OBJS) bin/server_main.o
	$(LINK) $(LINKFLAGS) $(OBJS) bin/server_main.o -o server_main -lrt

server_worker: $(OBJS) bin/server_worker.o
	$(LINK) $(LINKFLAGS) $(OBJS) bin/server_worker.o -o server_worker -lrt

bin/server_main.o: src/server_main.cpp
	$(CC) $(CCFLAGS) -c src/server_main.cpp -o bin/server_main.o -lrt

bin/server_worker.o: src/server_worker.cpp
	$(CC) $(CCFLAGS) -c src/server_worker.cpp -o bin/server_worker.o -lrt
	
bin/timer.o: src/timer.cpp
	$(CC) $(CCFLAGS) -c src/timer.cpp -o bin/timer.o -lrt

bin/server_manager.o: src/server_manager.cpp
	$(CC) $(CCFLAGS) -c src/server_manager.cpp -o bin/server_manager.o -lrt





bin/worker_thread.o: src/worker_thread.cpp
	$(CC) $(CCFLAGS) -c src/worker_thread.cpp -o bin/worker_thread.o -lrt

bin/subproblem.o: src/subproblem.cpp
	$(CC) $(CCFLAGS) -c src/subproblem.cpp -o bin/subproblem.o -lrt

bin/solution_manager.o: src/solution_manager.cpp
	$(CC) $(CCFLAGS) -c src/solution_manager.cpp -o bin/solution_manager.o -lrt

bin/segment.o: src/segment.cpp
	$(CC) $(CCFLAGS) -c src/segment.cpp -o bin/segment.o -lrt






bin/connection_worker.o: src/connection_worker.cpp
	$(CC) $(CCFLAGS) -c src/connection_worker.cpp -o bin/connection_worker.o -lrt
	
bin/connection_main.o: src/connection_main.cpp
	$(CC) $(CCFLAGS) -c src/connection_main.cpp -o bin/connection_main.o -lrt
	
	
	
	

bin/SocketPassive.o: src/SocketPassive.cpp
	$(CC) $(CCFLAGS) -c src/SocketPassive.cpp -o bin/SocketPassive.o -lrt

bin/SocketActive.o: src/SocketActive.cpp
	$(CC) $(CCFLAGS) -c src/SocketActive.cpp -o bin/SocketActive.o -lrt
	
bin/SocketBase.o: src/SocketBase.cpp
	$(CC) $(CCFLAGS) -c src/SocketBase.cpp -o bin/SocketBase.o -lrt
	
clean:
	rm bin/*.o
