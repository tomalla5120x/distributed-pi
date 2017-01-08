#include "connection_main.h"

#include <signal.h>
#include "solution_manager.h"


bool ConnectionMain::awaitingHello(Message message)
{
    if(message.getSequence() != lastRecvMessageSeq + 1
            || message.getTag() != MessageHello) {
        return true;
    }

    lastRecvMessageSeq = message.getSequence();

    sendSubproblem();

    return true;
}

bool ConnectionMain::awaitingWorkACK(Message message)
{
    auto seq = message.getSequence();
    if(seq != lastRecvMessageSeq + 1 || seq != lastRecvMessageSeq + 2) {
        return true;
    }

    lastRecvMessageSeq = seq;

    auto tag = message.getTag();
    if(tag == MessageACK) {
        responseTimer.unset();

        heartbeatTimer.set();

        solutionManager.assign(subproblemSegmentId, worker);

        stateHandler = &ConnectionMain::awaitingResult;
    } else if(tag == MessageResult) {
        solutionManager.markSolved(message.getSegmentID(), message.getPointsHit());
        sendSubproblem();
    }

    return true;
}

bool ConnectionMain::awaitingResult(Message message)
{
    auto seq = message.getSequence();
    if(seq != lastRecvMessageSeq + 1 || message.getTag() != MessageResult) {
        return true;
    }

    lastRecvMessageSeq = seq;

    solutionManager.markSolved(message.getSegmentID(), message.getPointsHit());
    sendSubproblem();

    return true;
}

bool ConnectionMain::standingBy(Message)
{
    // wiadomości wysłane na skutek timeoutu po stronie serwera roboczego
    // powinny zostać obsłużone w handleMessage()
    throw std::runtime_error("Worker cannot receive any messages while Standing By.");
}

void ConnectionMain::sendMessage(Message message, bool setTimer)
{
    socket.sendMessage(message, worker.ip, worker.port);
    lastMessageSent = message;

    if(setTimer) {
        responseTimer.set();
    }
}

void ConnectionMain::sendSubproblem()
{
    heartbeatTimer.unset();

    auto subproblem = solutionManager.pop(worker);
    if(subproblem == nullptr) {
        sendMessage(Message(MessageACK, lastRecvMessageSeq + 1), false);
        heartbeatTimer.set();

        stateHandler = &ConnectionMain::standingBy;

        return;
    }

    subproblemSegmentId = subproblem->getSegmentId();

    sendMessage(Message(MessageWork, lastRecvMessageSeq + 1, subproblem->getSegmentId(), subproblem->getPoints(), subproblem->getSide()));
    stateHandler = &ConnectionMain::awaitingWorkACK;
}

ConnectionMain::~ConnectionMain()
{
    responseTimer.unset();
    heartbeatTimer.unset();
}

ConnectionMain::ConnectionMain(SocketPassive& socket, SID worker) :
    socket(socket), worker(worker),
    responseTimer(timerSignal, responseTimeoutMs, true),
    heartbeatTimer(timerSignal, heartbeatTimeoutMs, true),
    solutionManager(SolutionManager::getInstance())
{
}

void ConnectionMain::startTimeout()
{
    responseTimer.set();
}

void ConnectionMain::stopTimeout()
{
    responseTimer.unset();
}

bool ConnectionMain::isTimeoutExpired() const
{
    return responseTimer.isRunning();
}

bool ConnectionMain::handleTimeout()
{
    repeatCount--;
    if(repeatCount <= 0) {
        if(stateHandler == &ConnectionMain::awaitingWorkACK || stateHandler == &ConnectionMain::awaitingResult) {
            solutionManager.unassign(subproblemSegmentId);
        }

        return false;
    }

    socket.sendMessage(lastMessageSent, worker.ip, worker.port);

    responseTimer.set();

    return true;
}

bool ConnectionMain::handleHeartbeatTimeout()
{
    if(stateHandler == &ConnectionMain::awaitingWorkACK || stateHandler == &ConnectionMain::awaitingResult) {
        solutionManager.unassign(subproblemSegmentId);
    }

    return false;
}

void ConnectionMain::startHeartbeatTimeout()
{
    heartbeatTimer.set();
}

void ConnectionMain::stopHeartbeatTimeout()
{
    heartbeatTimer.unset();
}

void ConnectionMain::resetHeartbeatTimeout()
{
    heartbeatTimer.set();
}

bool ConnectionMain::isHeartbeatTimeoutExpired() const
{
    return heartbeatTimer.isRunning();
}

bool ConnectionMain::handleMessage(Message message)
{
    if(message.getTag() == MessageHeartbeat && heartbeatTimer.isRunning()) {
        socket.sendMessage(MessageHeartbeatACK, worker.ip, worker.port);

        heartbeatTimer.set();
    }

    if(message.getTag() == MessageInterrupt) {
        if(stateHandler == &ConnectionMain::awaitingWorkACK || stateHandler == &ConnectionMain::awaitingResult) {
            solutionManager.unassign(subproblemSegmentId);
        }

        return false;
    }

    if(message.getSequence() < lastRecvMessageSeq) {
        return true;
    }

    if(message.getSequence() == lastRecvMessageSeq) {
        socket.sendMessage(lastMessageSent, worker.ip, worker.port);

        return true;
    }

    repeatCount = maxRepeats;

    return (this->*stateHandler)(message);
}

void ConnectionMain::assignSubproblem()
{
    if(stateHandler != &ConnectionMain::standingBy) {
        throw std::runtime_error("Worker can only be assigned a subproblem while Standing By.");
    }

    sendSubproblem();
}

void ConnectionMain::sendInterrupt()
{
	 socket.sendMessage(Message(MessageInterrupt), worker.ip, worker.port);
}

void ConnectionMain::sendClose()
{
	socket.sendMessage(Message(MessageClose, lastRecvMessageSeq+1), worker.ip, worker.port);
}

SID ConnectionMain::getSID() const
{
	return worker;
}

int ConnectionMain::getTimerSignal() {
	return timerSignal;
}