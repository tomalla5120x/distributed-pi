#ifndef CONNECTIONBASE_H
#define CONNECTIONBASE_H

#include "Message.h"

class ConnectionBase
{
public:
    virtual void startTimeout() = 0;
    virtual void stopTimeout() = 0;
    virtual bool isTimeoutExpired() const = 0;

    virtual void startHeartbeatTimeout() = 0;
    virtual void stopHeartbeatTimeout() = 0;
    virtual void resetHeartbeatTimeout() = 0;
    virtual bool isHeartbeatTimeoutExpired() const = 0;

    virtual bool handleMessage(Message message) = 0;
    virtual bool handleTimeout() = 0;
    virtual bool handleHeartbeatTimeout() = 0;
};

#endif
