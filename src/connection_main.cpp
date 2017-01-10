#include "connection_main.h"

#include <signal.h>
#include <sstream>
#include "solution_manager.h"
#include "easylogging++.h"


bool ConnectionMain::awaitingHello(Message message)
{
    if(message.getSequence() != nextSequence
            || message.getTag() != MessageHello) {
        CLOG(INFO, "connection") << logPreamble() << message.str() << " UNEXPECTED sequence number or tag. Message IGNORED";

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
    if(seq != nextSequence && seq != nextSequence + 1) {
        CLOG(INFO, "connection") << logPreamble() << message.str() << " UNXEPECTED sequence number or tag. Message IGNORED";

        return true;
    }

    lastMessageRecv = message;
    nextSequence = message.getSequence() + 1;

    auto tag = message.getTag();
    if(tag == MessageACK) {
        CLOG(INFO, "connection") << logPreamble() << "ACK RECEIVED. Subproblem ASSIGNED...";

        heartbeatTimer.set();
        solutionManager.assign(subproblemSegmentId, worker);

        CLOG(INFO, "connection") << logPreamble() << "TRANSITION to [AwaitingResult] state...";

        stateHandler = &ConnectionMain::awaitingResult;
    } else if(tag == MessageResult) {
    	solutionManager.assign(subproblemSegmentId, worker);
        solutionManager.markSolved(message.getSegmentID(), message.getPointsHit());
        sendSubproblem();
    }

    responseTimer.unset();

    return true;
}

bool ConnectionMain::awaitingResult(Message message)
{
    auto seq = message.getSequence();
    if(seq != nextSequence || message.getTag() != MessageResult) {
        CLOG(INFO, "connection") << logPreamble() << message.str() << " UNEXPECTED sequence number or tag. IGNORING message...";

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

    std::string text("Worker cannot receive any messages while Standing By.");
    LOG(FATAL) << logPreamble() << text;
    
    throw std::runtime_error(text);
}

void ConnectionMain::sendMessage(Message message, bool setTimer, bool resend)
{
    socket.sendMessage(message, worker.ip, worker.port);
    lastMessageSent = message;

    CLOG(INFO, "connection") << logPreamble() << message.str() << " SENT";

    if(setTimer) {
        CLOG(INFO, "connection") << logPreamble() << "Response timer SET";

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
        CLOG(INFO, "connection") << logPreamble() << "No subproblem to assign. TRANSITION to [StandingBy] state...";

        sendMessage(Message(MessageACK, nextSequence), false);
        heartbeatTimer.set();

        stateHandler = &ConnectionMain::standingBy;

        return;
    }

    subproblemSegmentId = subproblem->getSegmentId();

    sendMessage(Message(MessageWork, nextSequence, subproblem->getSegmentId(), subproblem->getPoints(), subproblem->getSide()));

    CLOG(INFO, "connection") << logPreamble() << "Subproblem FOUND. Subproblem SENT";
    CLOG(INFO, "connection") << logPreamble() << "TRANSITION to [AwaitingWorkACK] state...";

    stateHandler = &ConnectionMain::awaitingWorkACK;
}

std::string ConnectionMain::logPreamble()
{
    std::ostringstream oss;
    oss << "ConnectionMain(" << SocketBase::iptostr(worker.ip) << ", " << worker.port << ", nextseq = " << nextSequence << "): ";

    if(stateHandler == &ConnectionMain::awaitingHello) {
        oss << "[AwaitingHello] ";
    } else if(stateHandler == &ConnectionMain::awaitingWorkACK) {
        oss << "[AwaitingWorkACK] ";
    } else if(stateHandler == &ConnectionMain::awaitingResult) {
        oss << "[AwaitingResult] ";
    } else if(stateHandler == &ConnectionMain::standingBy) {
        oss << "[StandingBy] ";
    }

    return oss.str();
}

ConnectionMain::~ConnectionMain()
{
    responseTimer.unset();
    heartbeatTimer.unset();
    
    if(stateHandler == &ConnectionMain::awaitingWorkACK || stateHandler == &ConnectionMain::awaitingResult) {
        solutionManager.unassign(subproblemSegmentId);
    }
}

ConnectionMain::ConnectionMain(SocketPassive& socket, SID worker) :
    socket(socket), worker(worker),
    responseTimer(timerSignal, responseTimeoutMs, true),
    heartbeatTimer(heartbeatTimerSignal, heartbeatTimeoutMs, true),
    solutionManager(SolutionManager::getInstance())
{
    CLOG(INFO, "connection") << logPreamble() << "INSTANTIATED.";
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
    return !responseTimer.isRunning();
}

bool ConnectionMain::handleTimeout()
{
    repeatCount--;
    if(repeatCount <= 0) {
        if(stateHandler == &ConnectionMain::awaitingWorkACK || stateHandler == &ConnectionMain::awaitingResult) {
            solutionManager.unassign(subproblemSegmentId);
        }

        CLOG(INFO, "connection") << logPreamble() << "Response TIMEOUT occurred. NO REMAINING timeouts. CLOSING connection..." << repeatCount;

        return false;
    }

    sendMessage(lastMessageSent, true, true);

    CLOG(INFO, "connection") << logPreamble() << "Response TIMEOUT occurred. " << lastMessageSent.str() << " RESENT. Timeouts REMAINING: " << repeatCount;

    return true;
}

bool ConnectionMain::handleHeartbeatTimeout()
{
    if(stateHandler == &ConnectionMain::awaitingWorkACK || stateHandler == &ConnectionMain::awaitingResult) {
        solutionManager.unassign(subproblemSegmentId);
    }

    CLOG(INFO, "connection") << logPreamble() << "Heartbeat TIMEOUT occurred. CLOSING connection...";

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
    return !heartbeatTimer.isRunning();
}

bool ConnectionMain::handleMessage(Message message)
{
    CLOG(INFO, "connection") << logPreamble() << message.str() << " RECEIVED";

    auto tag = message.getTag();
    auto seq = message.getSequence();

    if(tag == MessageHeartbeat && heartbeatTimer.isRunning()) {
        socket.sendMessage(MessageHeartbeatACK, worker.ip, worker.port);
        heartbeatTimer.set();

        CLOG(INFO, "connection") << logPreamble() << message.str() << " Heartbeat RECEIVED. ACK sent. Heartbeat timer RESET";
        return true;
    }

    if(tag == MessageInterrupt) {
        if(stateHandler == &ConnectionMain::awaitingWorkACK || stateHandler == &ConnectionMain::awaitingResult) {
            solutionManager.unassign(subproblemSegmentId);
        }

        CLOG(INFO, "connection") << logPreamble() << message.str() << " RECEIVED. CLOSING connection...";

        return false;
    }

    // zignorowanie wiadomości o numerze sekw. niższym niż w ostatniej otrzymanej wiadomości
    if(seq < lastMessageRecv.getSequence()) {
        CLOG(INFO, "connection") << logPreamble() << "Message of too low sequence number RECEIVED. Message IGNORED";

        return true;
    }

    // ponowne wysłanie wiadomości w przypadku powtórnego otrzymania poprzednio otrzymanej wiadomości
    if(seq == lastMessageRecv.getSequence() && tag == lastMessageRecv.getTag()) {
        bool setTimer = lastMessageSent.getTag() != MessageACK;
        sendMessage(lastMessageSent, setTimer, true);

        CLOG(INFO, "connection") << logPreamble() << "Last received message RECEIVED AGAIN. Message RESENT";

        return true;
    }

    repeatCount = maxRepeats;

    return (this->*stateHandler)(message);
}

bool ConnectionMain::tryAssignSubproblem()
{
    if(stateHandler != &ConnectionMain::standingBy) {
        return false;
    }

    auto subproblem = solutionManager.pop(worker);
    if(subproblem == nullptr)
    	return false;
    	
    heartbeatTimer.unset();

    subproblemSegmentId = subproblem->getSegmentId();

    sendMessage(Message(MessageWork, nextSequence, subproblem->getSegmentId(), subproblem->getPoints(), subproblem->getSide()));
    stateHandler = &ConnectionMain::awaitingWorkACK;
    	
    return true;
}

void ConnectionMain::sendInterrupt()
{
	 socket.sendMessage(Message(MessageInterrupt), worker.ip, worker.port);

     CLOG(INFO, "connection") << logPreamble() << "MessageInterrupt SENT";
}

void ConnectionMain::sendClose()
{
    socket.sendMessage(Message(MessageClose, nextSequence), worker.ip, worker.port);

    CLOG(INFO, "connection") << logPreamble() << "MessageClose SENT";
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
