#include "Message.h"

#include <sstream>

Message::TypeToNameMap Message::typeToName = {
    { MessageACK, "MessageACK" },
    { MessageHello, "MessageHello" },
    { MessageWork, "MessageWork" },
    { MessageResult, "MessageResult" },
    { MessageClose, "MessageClose" },
    { MessageInterrupt, "MessageInterrupt" },
    { MessageHeartbeat, "MessageHeartbeat" },
    { MessageHeartbeatACK, "MessageHeartbeatACK" }
};

std::string Message::str()
{
    std::ostringstream oss;

    auto tag = getTag();
    auto seq = getSequence();

    oss << Message::typeToName[tag];

    if(tag == MessageInterrupt || tag == MessageHeartbeat || tag == MessageHeartbeatACK) {
        return oss.str();
    }

    oss << "(" << seq << ")";

    if(tag == MessageWork) {
        oss << " { segmentId = " << getSegmentID() << "; points = " << getPoints() << "; side = " << getSide() << " }";

        return oss.str();
    }

    if(tag == MessageResult) {
        oss << " { segmentId = " << getSegmentID() << "; pointsHit = " << getPoints() << " }";

        return oss.str();
    }

    return oss.str();
}
