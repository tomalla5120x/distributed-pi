#include "subproblem.h"
#include "segment.h"
#include <stdexcept>
using namespace std;

Subproblem::Subproblem(uint32_t id, uint32_t mSide, uint64_t nPoints)
{
    if(mSide == 0 || mSide > MAX_SIDE_VALUE)
    {
        throw runtime_error("Side value must be between 0 and MAX_VALUE");
    }
    if(nPoints == 0)
    {
        throw runtime_error("Points to choose at random number must be greater than 0");
    }
    segmentId = id;
    state = UNASSIGNED;
    side = mSide;
    points = nPoints;

    resetSIDAssignedTo();

}

void Subproblem::resetSIDAssignedTo()
{
    assignedTo.ip = 0;
    assignedTo.port = 0;
}

uint32_t Subproblem::getSegmentId()
{
    return segmentId;
}

uint32_t Subproblem::getSide()
{
    return side;
}

uint64_t Subproblem::getPoints()
{
    return points;
}

SubproblemState Subproblem::getState()
{
    return state;
}

SID Subproblem::getSIDAssignedTo()
{
    return assignedTo;
}

void Subproblem::setState(SubproblemState newState)
{
    state = newState;
}

void Subproblem::setSIDAssignedTo(SID worker)
{
    assignedTo = worker;
}
