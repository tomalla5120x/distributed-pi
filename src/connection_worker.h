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
    static const int timerSignal = SIGUSR1;
    static const int heartbeatExpectTimerSignal = SIGPOLL;
    static const int heartbeatDelayTimerSignal = SIGALRM;
    static const int responseTimeoutMs = 3000;
    static const int heartbeatTimeoutMs = 5000;
    static const int heartbeatMs = 500;
    static const int maxRepeats = 5;

    SocketActive& socket;

    Timer responseTimer;
    Timer heartbeatTimeoutTimer;
    Timer heartbeatTimer;

    int repeatCount = maxRepeats;

    Message lastMessageSent;
    Message lastMessageRecv;
    uint32_t nextSequence = 1;

    std::unique_ptr<WorkerThread> workerThread;

    bool (ConnectionWorker::* stateHandler)(Message message) = &ConnectionWorker::awaitingWorkOrACK;

    bool awaitingWorkOrACK(Message message);
    bool working(Message message);
    bool standingBy(Message message);

protected:
    void sendMessage(Message message, bool setTimer = true, bool resend = false);
    std::string logPreamble();

public:
    ConnectionWorker(SocketActive& socket);
    virtual ~ConnectionWorker();

    static int getTimerSignal();
    static int getHeartbeatExpectSignal();
    static int getHeartbeatDelaySignal();

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
    
    void handleHeartbeat();
};

#endif // SERVERCONNECTION_H
