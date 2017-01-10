#include "connection_worker.h"
#include "worker_thread.h"
#include "easylogging++.h"

bool ConnectionWorker::awaitingWorkOrACK(Message message)
{
    if(message.getTag() == MessageACK) {
        CLOG(INFO, "connection") << logPreamble() << "Going to [StandingBy] state...";

        stateHandler = &ConnectionWorker::standingBy;
    } else if(message.getTag() == MessageWork) {
        workerThread.reset(new WorkerThread(message.getSegmentID(), message.getSide(), message.getPoints()));
        workerThread.get()->start();

        CLOG(INFO, "connection") << logPreamble() << "Work RECEIVED. Worker thread STARTED";

        sendMessage(Message(MessageACK, nextSequence), false);

        CLOG(INFO, "connection") << logPreamble() << "TRANSITION to [Working] state...";

        stateHandler = &ConnectionWorker::working;
    }

    responseTimer.unset();

    CLOG(INFO, "connection") << logPreamble() << "ResponseTimer UNSET";

    heartbeatTimer.set();
    heartbeatTimeoutTimer.set();

    CLOG(INFO, "connection") << logPreamble() << "Heartbeat timers SET";

    return true;
}

bool ConnectionWorker::working(Message)
{
    return true;
}

bool ConnectionWorker::standingBy(Message message)
{
    if(message.getTag() != MessageWork) {
        CLOG(INFO, "connection") << logPreamble() << message.str() << " UNEXPECTED tag. Message IGNORED";

        return true;
    }

    workerThread.reset(new WorkerThread(message.getSegmentID(), message.getSide(), message.getPoints()));
    workerThread.get()->start();

    CLOG(INFO, "connection") << logPreamble() << "Work RECEIVED. Worker thread STARTED";

    sendMessage(Message(MessageACK, nextSequence), false);

    CLOG(INFO, "connection") << logPreamble() << "Heartbeat timers SET";

    heartbeatTimer.set();
    heartbeatTimeoutTimer.set();

    CLOG(INFO, "connection") << logPreamble() << "TRANSITION to [Working] state...";

    stateHandler = &ConnectionWorker::working;
    	
    return true;
}

void ConnectionWorker::sendMessage(Message message, bool setTimer, bool resend)
{
    socket.send(message);
    lastMessageSent = message;

    CLOG(INFO, "connection") << logPreamble() << message.str() << " SENT";

    if(setTimer) {
        responseTimer.set();

        CLOG(INFO, "connection") << logPreamble() << "Response timer SET";
    }

    if(!resend) {
        nextSequence++;
    }
}

std::string ConnectionWorker::logPreamble()
{
    std::ostringstream oss;
    oss << "ConnectionWorker(nextseq = " << nextSequence << "): ";

    if(stateHandler == &ConnectionWorker::awaitingWorkOrACK) {
        oss << "[AwaitingWorkOrACK] ";
    } else if(stateHandler == &ConnectionWorker::working) {
        oss << "[Working] ";
    } else if(stateHandler == &ConnectionWorker::standingBy) {
        oss << "[ConnectionWorker] ";
    }

    return oss.str();
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
    return !responseTimer.isRunning();
}

void ConnectionWorker::startHeartbeatTimeout()
{
    heartbeatTimer.set();
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
    return !heartbeatTimer.isRunning();
}

bool ConnectionWorker::handleMessage(Message message)
{
    CLOG(INFO, "connection") << logPreamble() << message.str() << " RECEIVED";

    auto tag = message.getTag();
    auto seq = message.getSequence();

    if(tag == MessageHeartbeatACK && heartbeatTimeoutTimer.isRunning()) {
        heartbeatTimeoutTimer.set();

        CLOG(INFO, "connection") << logPreamble() << message.str() << " Heartbeat RECEIVED. ACK sent. HeartbeatTimeout timer RESET";

        return true;
    }

    if(tag == MessageInterrupt) {
        CLOG(INFO, "connection") << logPreamble() << message.str() << " RECEIVED. TERMINATING...";

        return false;
    }

    // zignorowanie wiadomości o numerze sekw. niższym niż w ostatniej otrzymanej wiadomości
    if(seq < lastMessageRecv.getSequence()) {
        CLOG(INFO, "connection") << logPreamble() << "Message of too low sequence number RECEIVED. Message IGNORED";

        return true;
    }

    // ponowne wysłanie wiadomości w przypadku powtórnego otrzymania poprzednio otrzymanej wiadomości
    if(seq == lastMessageRecv.getSequence()) {
        bool setTimer = lastMessageSent.getTag() != MessageACK;
        sendMessage(lastMessageSent, setTimer, true);

        CLOG(INFO, "connection") << logPreamble() << "Last received message RECEIVED AGAIN. Message RESENT";

        return true;
    }

    if(seq != nextSequence) {
        CLOG(INFO, "connection") << logPreamble() << message.str() << " UNEXPECTED sequence. Message IGNORED";

        return true;
    }

    if(message.getTag() == MessageClose) {
        CLOG(INFO, "connection") << logPreamble() << message.str() << " RECEIVED. TERMINATING...";

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
        CLOG(INFO, "connection") << logPreamble() << "Response TIMEOUT occurred. NO REMAINING timeouts. TERMINATING...";
        socket.send(Message(MessageInterrupt));

        return false;
    }

    sendMessage(lastMessageSent, true, true);

    CLOG(INFO, "connection") << logPreamble() << "Response TIMEOUT occurred. " << lastMessageSent.str() << " RESENT. Timeouts REMAINING: " << repeatCount;
	
	return true;
}

bool ConnectionWorker::handleHeartbeatTimeout()
{
    if(heartbeatTimer.isRunning()) {
        CLOG(INFO, "connection") << logPreamble() << "Heartbeat TIMEOUT occurred. TERMINATING...";

        return false;
    }

    return true;
}

void ConnectionWorker::handleHeartbeat()
{
    socket.send(Message(MessageHeartbeat));

    CLOG(INFO, "connection") << logPreamble() << "Heartbeat occurred. " << Message(MessageHeartbeat).str() << " SENT";
}

void ConnectionWorker::sendResult()
{
    if(stateHandler != &ConnectionWorker::working) {
        std::string text;
        //CLOG(INFO, "connection") << logPreamble() << text;

        throw std::logic_error("Result may only be sent while in Working state.");
    }

    CLOG(INFO, "connection") << logPreamble() << "Computation FINISHED. SENDING result...";

    auto result = workerThread.get()->getResult();
    sendMessage(Message(MessageResult, nextSequence, result.segmentId, result.pointsHit));

    heartbeatTimeoutTimer.unset();
    heartbeatTimer.unset();

    CLOG(INFO, "connection") << logPreamble() << "Heartbeat timers UNSET";

    stateHandler = &ConnectionWorker::awaitingWorkOrACK;

    CLOG(INFO, "connection") << logPreamble() << "TRANSITION to [AwaitingWorkOrACK] state...";
}

void ConnectionWorker::sendInterrupt() {
	socket.send(Message(MessageInterrupt));
	
    CLOG(INFO, "connection") << logPreamble() << "Interrupt occured. " << Message(MessageInterrupt).str() << " SENT";
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
