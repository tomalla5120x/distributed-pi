#include "connection_main.h"

#include <signal.h>
#include "solution_manager.h"


bool ConnectionMain::awaitingHello(Message message)
{
    if(message.getSequence() != nextSequence
            || message.getTag() != MessageHello) {
        return true;
    }

    lastMessageRecv = message;
    nextSequence = message.getSequence() + 1;

    sendSubproblem();

    return true;
}

bool ConnectionMain::awaitingWorkACK(Message message)
{
    auto seq = message.getSequence();
    if(seq != nextSequence || seq != nextSequence + 1) {
        return true;
    }

    lastMessageRecv = message;
    nextSequence = message.getSequence() + 1;

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
    if(seq != nextSequence || message.getTag() != MessageResult) {
        return true;
    }

    lastMessageRecv = message;
    nextSequence = message.getSequence() + 1;

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

void ConnectionMain::sendMessage(Message message, bool setTimer, bool resend)
{
    socket.sendMessage(message, worker.ip, worker.port);
    lastMessageSent = message;

    if(setTimer) {
        responseTimer.set();
    }

    if(!resend) {
        nextSequence++;
    }
}

void ConnectionMain::sendSubproblem()
{
    heartbeatTimer.unset();

    auto subproblem = solutionManager.pop(worker);
    if(subproblem == nullptr) {
        sendMessage(Message(MessageACK, nextSequence), false);
        heartbeatTimer.set();

        stateHandler = &ConnectionMain::standingBy;

        return;
    }

    subproblemSegmentId = subproblem->getSegmentId();

    sendMessage(Message(MessageWork, nextSequence, subproblem->getSegmentId(), subproblem->getPoints(), subproblem->getSide()));
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

    sendMessage(lastMessageSent, true, true);

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
    auto tag = message.getTag();
    auto seq = message.getSequence();

    if(tag == MessageHeartbeat && heartbeatTimer.isRunning()) {
        socket.sendMessage(MessageHeartbeatACK, worker.ip, worker.port);

        heartbeatTimer.set();
    }

    if(tag == MessageInterrupt) {
        if(stateHandler == &ConnectionMain::awaitingWorkACK || stateHandler == &ConnectionMain::awaitingResult) {
            solutionManager.unassign(subproblemSegmentId);
        }

        return false;
    }

    // zignorowanie wiadomości o numerze sekw. niższym niż w ostatniej otrzymanej wiadomości
    if(seq < lastMessageRecv.getSequence()) {
        return true;
    }

    // ponowne wysłanie wiadomości w przypadku powtórnego otrzymania poprzednio otrzymanej wiadomości
    if(seq == lastMessageRecv.getSequence() && tag == lastMessageRecv.getTag()) {
        bool setTimer = lastMessageSent.getTag() != MessageACK;
        sendMessage(lastMessageSent, setTimer, true);

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
    socket.sendMessage(Message(MessageClose, nextSequence), worker.ip, worker.port);
}

SID ConnectionMain::getSID() const
{
	return worker;
}

int ConnectionMain::getTimerSignal()
{
    return timerSignal;
}

int ConnectionMain::getHeartbeatTimerSignal()
{
    return heartbeatTimerSignal;
}
