#include "worker_thread.h"
#include "segment.h"
#include <chrono>
#include <stdexcept>
using namespace std;

WorkerThread::WorkerThread(uint32_t id, uint32_t mSide, uint64_t nPoints)
{
    if(mSide == 0)
    {
        throw runtime_error("Side value must be greater than 0");
    }
    if(nPoints == 0)
    {
        throw runtime_error("Points to choose at random number must be greater than 0");
    }
    segmentId = id;
    side = mSide;
    points = nPoints;
    pointsHit = 0;
    running = false;
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    default_random_engine generator(seed);
    engine = generator;
}

void WorkerThread::count()
{
    Segment currentSegment;
    currentSegment.setSegmentId(segmentId);
    currentSegment.setSide(side);
    uint32_t radius = K * side;
    uint32_t xLeft = currentSegment.getXLeft();
    uint32_t xRight = currentSegment.getXRight();
    uint32_t yBottom = currentSegment.getYBottom();
    uint32_t yTop = currentSegment.getYTop();

    uniform_real_distribution<double> xDistribution(xLeft, xRight), yDistribution(yBottom, yTop);
    double x, y;
    for(uint64_t i = 0; i < points; i++)
    {
        x = xDistribution(engine);
        y = yDistribution(engine);
        if(isPointInsideCircle(x, y, radius))
        {
            pointsHit++;
        }
    }
}

void* WorkerThread::threadEntry(void* myThis)
{
    ((WorkerThread*) myThis)->count();

    struct sigaction action;
    action.sa_handler = signalHandler; //pewnie petla serwera roboczego
    action.sa_flags = 0;
    if(sigemptyset(&action.sa_mask) != 0)
    {
        throw runtime_error("Error setting sigemptyset");
    }
    if(sigaction(SIGUSR2, &action, NULL) != 0)
    {
        throw runtime_error("Error setting sigaction");
    }
    if(raise(SIGUSR2) != 0)
    {
        throw runtime_error("Error sendind signal from calculating thread");
    }

    return nullptr;
}

void WorkerThread::start()
{
    if(!running)
    {
        running = true;
        if(pthread_create(&thread, NULL, threadEntry, this) != 0)
        {
            running = false;
            throw runtime_error("Error creating calculating thread");
        }
    }

}

void WorkerThread::stop()
{
    running = false;
    if(pthread_kill(thread, SIGINT) != 0)
    {
        throw runtime_error("Error killing calculating process");
    }

}

bool WorkerThread::isRunning()
{
    return running;
}

WorkerResult WorkerThread::getResult()
{
    if(pthread_join(thread, NULL) != 0)
    {
        throw runtime_error("Error joining calculating thread");
    }
    WorkerResult result;
    result.segmentId = segmentId;
    result.pointsHit = pointsHit;
    running = false;
    return result;
}
