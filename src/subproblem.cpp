#include "subproblem.h"
#include <stdexcept>
using namespace std;

Subproblem::Subproblem(uint32_t id, uint32_t mSide, uint64_t nPoints)
{
    if(mSide == 0)
    {
        throw runtime_error("Side value must be greater than 0");
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
    assignedTo = 0; //do weryfikacji przez Tomka, ale jakiœ reset by siê przyda³
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
