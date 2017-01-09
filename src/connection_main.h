#ifndef CONNECTIONMAIN_H
#define CONNECTIONMAIN_H

#include "server_manager.h"
#include "connection_base.h"
#include "SocketPassive.h"
#include "timer.h"

class SolutionManager;

class ConnectionMain : ConnectionBase
{
    static const int timerSignal = SIGUSR1;
    static const int heartbeatTimerSignal = SIGPOLL;
    static const int responseTimeoutMs = 3000;
    static const int heartbeatTimeoutMs = 3000;
    static const int maxRepeats = 5;

    SocketPassive& socket;
    SID worker;

    Timer responseTimer;
    Timer heartbeatTimer;

    int repeatCount = maxRepeats;

    Message lastMessageSent;
    Message lastMessageRecv;
    uint32_t nextSequence = 1;

    SolutionManager& solutionManager;
    uint32_t subproblemSegmentId;
    
    bool (ConnectionMain::* stateHandler)(Message message) = &ConnectionMain::awaitingHello;
    
    bool awaitingHello(Message message);
    bool awaitingWorkACK(Message message);
    bool awaitingResult(Message message);
    bool standingBy(Message message);

protected:
    void sendMessage(Message message, bool setTimer = true, bool resend = false);
    void sendSubproblem();

public:
    ConnectionMain(SocketPassive& socket, SID worker);
    virtual ~ConnectionMain();

    static int getTimerSignal();
    static int getHeartbeatTimerSignal();

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

    bool tryAssignSubproblem();
    void sendInterrupt();
    void sendClose();
    SID getSID() const;
};


#endif
