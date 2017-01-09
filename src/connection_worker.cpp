#include "connection_worker.h"
#include "worker_thread.h"
#include "easylogging++.h"

bool ConnectionWorker::awaitingWorkOrACK(Message message)
{
    if(message.getTag() == MessageACK) {
        stateHandler = &ConnectionWorker::standingBy;

        LOG(INFO) << logPreamble() << "Going to [StandingBy] state...";
    } else if(message.getTag() == MessageWork) {
        workerThread.reset(new WorkerThread(message.getSegmentID(), message.getSide(), message.getPoints()));
        workerThread.get()->start();

        LOG(INFO) << logPreamble() << "Work RECEIVED. Worker thread STARTED";

        sendMessage(Message(MessageACK, nextSequence), false);

        stateHandler = &ConnectionWorker::working;

        LOG(INFO) << logPreamble() << "Going to [Working] state...";
    }

    responseTimer.unset();

    LOG(INFO) << logPreamble() << "ResponseTimer UNSET";

    heartbeatTimer.set();
    heartbeatTimeoutTimer.set();

    LOG(INFO) << logPreamble() << "Heartbeat timers SET";

    return true;
}

bool ConnectionWorker::working(Message)
{
    return true;
}

bool ConnectionWorker::standingBy(Message message)
{
    if(message.getTag() != MessageWork) {
        LOG(INFO) << logPreamble() << message.str() << "UNEXPECTED tag. Message IGNORED";

        return true;
    }

    workerThread.reset(new WorkerThread(message.getSegmentID(), message.getSide(), message.getPoints()));
    workerThread.get()->start();

    LOG(INFO) << logPreamble() << "Work RECEIVED. Worker thread STARTED";

    sendMessage(Message(MessageACK, nextSequence), false);

    LOG(INFO) << logPreamble() << "Heartbeat timers SET";

    heartbeatTimer.set();
    heartbeatTimeoutTimer.set();

    stateHandler = &ConnectionWorker::working;

    LOG(INFO) << logPreamble() << "Going to [Working] state...";
	
	return true;
}

void ConnectionWorker::sendMessage(Message message, bool setTimer, bool resend)
{
    socket.send(message);
    lastMessageSent = message;

    LOG(INFO) << logPreamble() << message.str() << "SENT";

    if(setTimer) {
        responseTimer.set();

        LOG(INFO) << logPreamble() << "Response timer SET";
    }

    if(!resend) {
        nextSequence++;
    }
}

std::string ConnectionWorker::logPreamble()
{
    std::ostringstream oss;
    oss << "ConnectionWorker: ";

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
    LOG(INFO) << logPreamble() << message.str() << " RECEIVED";

    auto tag = message.getTag();
    auto seq = message.getSequence();

    if(tag == MessageHeartbeatACK && heartbeatTimeoutTimer.isRunning()) {
        heartbeatTimeoutTimer.set();

        LOG(INFO) << logPreamble() << message.str() << "Heartbeat RECEIVED. ACK sent. HeartbeatTimeout timer RESET";

        return true;
    }

    if(tag == MessageInterrupt) {
        LOG(INFO) << logPreamble() << message.str() << "RECEIVED. TERMINATING...";

        return false;
    }

    // zignorowanie wiadomości o numerze sekw. niższym niż w ostatniej otrzymanej wiadomości
    if(seq < lastMessageRecv.getSequence()) {
        LOG(INFO) << logPreamble() << "Message of too low sequence number RECEIVED. Message IGNORED";

        return true;
    }

    // ponowne wysłanie wiadomości w przypadku powtórnego otrzymania poprzednio otrzymanej wiadomości
    if(seq == lastMessageRecv.getSequence()) {
        bool setTimer = lastMessageSent.getTag() != MessageACK;
        sendMessage(lastMessageSent, setTimer, true);

        LOG(INFO) << logPreamble() << "Last received message RECEIVED AGAIN. Message RESENT";

        return true;
    }

    if(seq != nextSequence) {
        LOG(INFO) << logPreamble() << message.str() << "UNEXPECTED sequence. Message IGNORED";

        return true;
    }

    if(message.getTag() == MessageClose) {
        LOG(INFO) << logPreamble() << message.str() << "RECEIVED. TERMINATING...";

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
        LOG(INFO) << logPreamble() << "Response TIMEOUT occurred. NO REMAINING timeouts. TERMINATING..." << repeatCount;

        socket.send(MessageInterrupt);

        return false;
    }

    sendMessage(lastMessageSent, true, true);

    LOG(INFO) << logPreamble() << "Response TIMEOUT occurred. " << lastMessageSent << " RESENT. Timeouts REMAINING: " << repeatCount;
	
	return true;
}

bool ConnectionWorker::handleHeartbeatTimeout()
{
    if(heartbeatTimer.isRunning()) {
        LOG(INFO) << logPreamble() << "Heartbeat TIMEOUT occurred. TERMINATING...";

        return false;
    }

    return true;
}

void ConnectionWorker::handleHeartbeat()
{
    socket.send(Message(MessageHeartbeat));

    LOG(INFO) << logPreamble() << "Heartbeat occurred. " << Message(MessageHeartbeat) << " SENT";
}

void ConnectionWorker::sendResult()
{
    if(stateHandler != &ConnectionWorker::working) {
        std::string text;
        LOG(INFO) << logPreamble() << text;

        throw std::logic_error("Result may only be sent while in Working state.");
    }

    LOG(INFO) << logPreamble() << "Computation FINISHED. SENDING result...";

    auto result = workerThread.get()->getResult();
    sendMessage(Message(MessageResult, nextSequence, result.segmentId, result.pointsHit));

    heartbeatTimeoutTimer.unset();
    heartbeatTimer.unset();

    LOG(INFO) << logPreamble() << "Heartbeat timers UNSET";

    stateHandler = &ConnectionWorker::awaitingWorkOrACK;

    LOG(INFO) << logPreamle() << "Going to [AwaitingWorkOrACK] state...";
}

void ConnectionWorker::sendInterrupt() {
	socket.send(Message(MessageInterrupt));
	
	LOG(INFO) << logPreamble() << "Interrupt occured. " << Message(MessageInterrupt) << " SENT";
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
