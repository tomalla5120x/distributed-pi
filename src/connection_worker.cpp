#include "connection_worker.h"
#include "worker_thread.h"

bool ConnectionWorker::awaitingWorkOrACK(Message message)
{
    if(message.getTag() == MessageACK) {
        stateHandler = &ConnectionWorker::standingBy;
    } else if(message.getTag() == MessageWork) {
        workerThread.reset(new WorkerThread(message.getSegmentID(), message.getSide(), message.getPoints()));
        workerThread.get()->start();

        sendMessage(MessageACK, false);

        stateHandler = &ConnectionWorker::working;
    }

    heartbeatMode = DELAY;
    heartbeatTimer.set();

    return true;
}

bool ConnectionWorker::working(Message)
{
    return true;
}

bool ConnectionWorker::standingBy(Message message)
{
    if(message.getTag() != MessageWork) {
        return true;
    }

    workerThread.reset(new WorkerThread(message.getSegmentID(), message.getSide(), message.getPoints()));
    workerThread.get()->start();

    sendMessage(MessageACK);

    heartbeatMode = DELAY;
    heartbeatTimer.set();

    stateHandler = &ConnectionWorker::working;
	
	return true;
}

void ConnectionWorker::sendMessage(Message message, bool setTimer)
{
    socket.send(message);
    lastMessageSent = message;

    if(setTimer) {
        responseTimer.set();
    }
}

ConnectionWorker::ConnectionWorker(SocketActive &socket)
    : socket(socket),
      responseTimer(timerSignal, responseTimeoutMs, true),
      heartbeatTimer(timerSignal, heartbeatTimeoutMs, true)
{
    Message hello(MessageHello, lastRecvMessageSeq + 1);
    sendMessage(hello);
}

ConnectionWorker::~ConnectionWorker()
{
    responseTimer.unset();
    heartbeatTimer.unset();
}

void ConnectionWorker::startTimeout()
{
    responseTimer.set();
}

void ConnectionWorker::stopTimeout()
{
    responseTimer.unset();
}

bool ConnectionWorker::isTimeoutExpired() const
{
    return responseTimer.isRunning();
}

void ConnectionWorker::startHeartbeatTimeout()
{
    heartbeatTimer.isRunning();
}

void ConnectionWorker::stopHeartbeatTimeout()
{
    heartbeatTimer.unset();
}

void ConnectionWorker::resetHeartbeatTimeout()
{
    heartbeatTimer.set();
}

bool ConnectionWorker::isHeartbeatTimeoutExpired() const
{
    return heartbeatTimer.isRunning();
}

bool ConnectionWorker::handleMessage(Message message)
{
    if(message.getTag() == MessageHeartbeatACK && heartbeatTimer.isRunning() && heartbeatMode == EXPECT) {
        heartbeatRepeatCount = maxHeartbeatRepeats;

        heartbeatMode = DELAY;
        heartbeatTimer.set();

        return true;
    }

    if(message.getTag() == MessageInterrupt || message.getTag() == MessageClose) {
        return false;
    }

    auto seq = message.getSequence();

    if(seq < lastRecvMessageSeq) {
        return true;
    }

    if(seq == lastRecvMessageSeq) {
        sendMessage(lastMessageSent);

        return true;
    }

    if(seq != lastRecvMessageSeq + 1) {
        return true;
    }

    repeatCount = maxRepeats;

    return (this->*stateHandler)(message);
}

bool ConnectionWorker::handleTimeout()
{
    repeatCount--;
    if(repeatCount <= 0) {
        socket.send(MessageInterrupt);

        return false;
    }

    socket.send(lastMessageSent);

    responseTimer.set();
	
	return true;
}

bool ConnectionWorker::handleHeartbeatTimeout()
{
    if(heartbeatMode == DELAY) {
        socket.send(MessageHeartbeat);

        heartbeatMode = EXPECT;
        heartbeatTimer.set();

        return true;
    }

    heartbeatRepeatCount--;

    return heartbeatRepeatCount > 0;
}

void ConnectionWorker::sendResult()
{
    if(stateHandler != &ConnectionWorker::working) {
        throw std::logic_error("Result may only be sent while in Working state.");
    }

    auto result = workerThread.get()->getResult();
    sendMessage(Message(MessageResult, lastRecvMessageSeq + 1, result.segmentId, result.pointsHit));

    stateHandler = &ConnectionWorker::awaitingWorkOrACK;
}

int ConnectionWorker::getTimerSignal() {
	return timerSignal;
}