#include "connection_worker.h"
#include "worker_thread.h"

bool ConnectionWorker::awaitingWorkOrACK(Message message)
{
    if(message.getTag() == MessageACK) {
        stateHandler = &ConnectionWorker::standingBy;
    } else if(message.getTag() == MessageWork) {
        workerThread.reset(new WorkerThread(message.getSegmentID(), message.getSide(), message.getPoints()));
        workerThread.get()->start();

        sendMessage(Message(MessageACK, nextSequence), false);

        stateHandler = &ConnectionWorker::working;
    }

    heartbeatTimer.set();
    heartbeatTimeoutTimer.set();

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

    sendMessage(Message(MessageACK, nextSequence), false);

    heartbeatTimer.set();
    heartbeatTimeoutTimer.set();

    stateHandler = &ConnectionWorker::working;
	
	return true;
}

void ConnectionWorker::sendMessage(Message message, bool setTimer, bool resend)
{
    socket.send(message);
    lastMessageSent = message;

    if(setTimer) {
        responseTimer.set();
    }

    if(!resend) {
        nextSequence++;
    }
}

ConnectionWorker::ConnectionWorker(SocketActive &socket)
    : socket(socket),
      responseTimer(timerSignal, responseTimeoutMs, true),
      heartbeatTimeoutTimer(heartbeatExpectTimerSignal, heartbeatTimeoutMs, true),
      heartbeatTimer(heartbeatDelayTimerSignal, heartbeatMs, false)
{
    Message hello(MessageHello, nextSequence);
    sendMessage(hello);
}

ConnectionWorker::~ConnectionWorker()
{
    responseTimer.unset();
    heartbeatTimeoutTimer.unset();
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
    auto tag = message.getTag();
    auto seq = message.getSequence();

    if(tag == MessageHeartbeatACK && heartbeatTimeoutTimer.isRunning()) {
        heartbeatTimeoutTimer.set();

        return true;
    }

    if(tag == MessageInterrupt) {
        return false;
    }

    // zignorowanie wiadomości o numerze sekw. niższym niż w ostatniej otrzymanej wiadomości
    if(seq < lastMessageRecv.getSequence()) {
        return true;
    }

    // ponowne wysłanie wiadomości w przypadku powtórnego otrzymania poprzednio otrzymanej wiadomości
    if(seq == lastMessageRecv.getSequence()) {
        bool setTimer = lastMessageSent.getTag() != MessageACK;
        sendMessage(lastMessageSent, setTimer, true);

        return true;
    }

    if(seq != nextSequence) {
        return true;
    }

    if(message.getTag() == MessageClose) {
        return false;
    }

    nextSequence = message.getSequence() + 1;
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

    sendMessage(lastMessageSent, true, true);
	
	return true;
}

bool ConnectionWorker::handleHeartbeatTimeout()
{
    if(heartbeatTimer.isRunning()) {
        return false;
    }

    return true;
}

bool ConnectionWorker::handleHeartbeat()
{
    socket.send(MessageHeartbeat);
}

void ConnectionWorker::sendResult()
{
    if(stateHandler != &ConnectionWorker::working) {
        throw std::logic_error("Result may only be sent while in Working state.");
    }

    auto result = workerThread.get()->getResult();
    sendMessage(Message(MessageResult, nextSequence, result.segmentId, result.pointsHit));

    heartbeatTimeoutTimer.unset();
    heartbeatTimer.unset();

    stateHandler = &ConnectionWorker::awaitingWorkOrACK;
}

int ConnectionWorker::getTimerSignal()
{
    return timerSignal;
}

int ConnectionWorker::getHeartbeatExpectSignal()
{
    return heartbeatExpectTimerSignal;
}

int ConnectionWorker::getHeartbeatDelaySignal()
{
    return heartbeatDelayTimerSignal;
}
