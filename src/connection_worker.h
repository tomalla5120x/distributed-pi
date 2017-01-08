#ifndef CONNECTIONWORKER_H
#define CONNECTIONWORKER_H

#include <memory>

#include "connection_base.h"
#include "SocketActive.h"
#include "Message.h"
#include "timer.h"
#include "worker_thread.h"

class ConnectionWorker : ConnectionBase
{
    enum HeartbeatMode {
        DELAY,
        EXPECT
    };

    static const int timerSignal = SIGUSR1;
    static const int responseTimeoutMs = 3000;
    static const int heartbeatTimeoutMs = 500;
    static const int maxRepeats = 5;
    static const int maxHeartbeatRepeats = 10;

    SocketActive& socket;

    Timer responseTimer;
    Timer heartbeatTimer;
    HeartbeatMode heartbeatMode = DELAY;

    int repeatCount = maxRepeats;
    int heartbeatRepeatCount = maxHeartbeatRepeats;

    Message lastMessageSent;
    uint32_t lastRecvMessageSeq = 0;

    std::unique_ptr<WorkerThread> workerThread;

    bool (ConnectionWorker::* stateHandler)(Message message) = &ConnectionWorker::awaitingWorkOrACK;

    bool awaitingWorkOrACK(Message message);
    bool working(Message message);
    bool standingBy(Message message);

protected:
    void sendMessage(Message message, bool setTimer = true);

public:
    ConnectionWorker(SocketActive& socket);
    virtual ~ConnectionWorker();

    void startTimeout() override;
    void stopTimeout() override;
    bool isTimeoutExpired() const override;

    void startHeartbeatTimeout() override;
    void stopHeartbeatTimeout() override;
    void resetHeartbeatTimeout() override;
    bool isHeartbeatTimeoutExpired() const override;

    bool handleMessage(Message message) override;
    bool handleTimeout() override;
    bool handleHeartbeatTimeout() override;

    void sendResult();
    void sendInterrupt();
    
    static int getTimerSignal();
};

#endif // SERVERCONNECTION_H
