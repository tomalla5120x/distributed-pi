#include <random>
#include <pthread.h>
#include <signal.h>
#include <iostream>
#ifndef WORKER_THREAD_H
#define WORKER_THREAD_H
//void signalHandler(int sig);

typedef struct result
{
    uint32_t segmentId;
    uint64_t pointsHit;
} WorkerResult;

class WorkerThread
{
private:
    static const int threadSignal = SIGUSR2;
	
    uint32_t segmentId;
    uint32_t side;
    uint64_t points; //liczba punktow do wylosowania
    uint64_t pointsHit; //wynik
    bool running;
    std::default_random_engine engine;
    pthread_t thread;
    static void* threadEntry(void* myThis);
    //wywolywana przez watek obliczeniowy
    void count();
public:
    WorkerThread(uint32_t id, uint32_t mSide, uint64_t nPoints);
    void start();
    void stop();
    bool isRunning();
    WorkerResult getResult();
    
    static int getThreadSignal();
};

#endif // WORKER_THREAD_H
