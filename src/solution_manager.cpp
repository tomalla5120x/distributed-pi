#include "solution_manager.h"
#include <stdexcept>

SolutionManager::SolutionManager(uint32_t mSide, uint64_t nPoints, uint32_t digitsAfterDot)
{
    if(mSide == 0)
    {
        throw runtime_error("Side value must be greater than 0");
    }
    if(nPoints == 0)
    {
        throw runtime_error("Points to choose at random number must be greater than 0");
    }
    if(digitsAfterDot == 0)
    {
        throw runtime_error("Digits after dot number must be greater than 0");
    }

    side = mSide;
    points = nPoints;
    squareSegments = new Segment[side * side];
    this->digitsAfterDot = digitsAfterDot;
}

SolutionManager::~SolutionManager()
{
    delete [] squareSegments;
}

void SolutionManager::initialize()
{
    uint32_t radius = K * side;
    bool isResultKnown = false;
    uint32_t segmentId = 0;
    for(uint32_t i = 0; i < side * side; i++)
    {
        segmentId = i + 1;
        squareSegments[i].setSegmentId(segmentId);
        squareSegments[i].setSide(side);
        isResultKnown = squareSegments[i].isResultKnown(radius);
        if(isResultKnown)
        {
            if(squareSegments[i].isWholeInsideCircle(radius))
            {
                squareSegments[i].setPointsHit(points);
            } else {
                squareSegments[i].setPointsHit(0);
            }
        } else {
            Subproblem* subproblem = new Subproblem(segmentId, side, points);
            subproblems.push_back(subproblem);
        }
    }
}

void SolutionManager::markSolved(uint32_t segmentId, uint64_t pointsHit)
{
    list<Subproblem*>::iterator it;
    for(it = subproblems.begin(); it != subproblems.end(); it++)
    {
        if((*it)->getSegmentId() == segmentId)
        {
            if((*it)->getState() != ASSIGNED)
            {
                throw runtime_error("Attempt to mark as SOLVED problem which is not ASSIGNED");
            }
            (*it)->setState(SOLVED);
            (*it)->resetSIDAssignedTo();
            squareSegments[segmentId - 1].setPointsHit(pointsHit);
            return;
        }
    }
    throw runtime_error("Attempt to mark as SOLVED problem which is not in the subproblems list");
}

void SolutionManager::assign(uint32_t segmentId, SID worker)
{
    list<Subproblem*>::iterator it;
    for(it = subproblems.begin(); it != subproblems.end(); it++)
    {
        if((*it)->getSegmentId() == segmentId)
        {
            if((*it)->getState() != ASSIGNED_PENDING)
            {
                throw runtime_error("Attempt to ASSIGN problem which is not ASSIGNED_PENDING"); //mo¿e jeszcze sprawdziæ, czy próbuje siê go przypisaæ temu samemu workerowi
            }
            (*it)->setState(ASSIGNED);
            (*it)->setSIDAssignedTo(worker);
            return;
        }
    }
    throw runtime_error("Attempt to ASSIGN problem which is not in the subproblems list");
}

void SolutionManager::unassign(uint32_t segmentId)
{
    list<Subproblem*>::iterator it;
    for(it = subproblems.begin(); it != subproblems.end(); it++)
    {
        if((*it)->getSegmentId() == segmentId)
        {
            if((*it)->getState() != ASSIGNED_PENDING && (*it)->getState() != ASSIGNED)
            {
                throw runtime_error("Attempt to UNASSIGN problem which is neither ASSIGNED_PENDING nor ASSIGNED");
            }
            (*it)->setState(UNASSIGNED);
            (*it)->resetSIDAssignedTo();
            return;
        }
    }
    throw runtime_error("Attempt to UNASSIGN problem which is not in the subproblems list");
}

bool SolutionManager::solved()
{
    list<Subproblem*>::iterator it;
    for(it = subproblems.begin(); it != subproblems.end(); it++)
    {
        if((*it)->getState() != SOLVED)
        {
            return false;
        }
    }
    return true;
}
string SolutionManager::getResult()
{

}
Subproblem* SolutionManager::pop(SID worker)
{
    list<Subproblem*>::iterator it;
    for(it = subproblems.begin(); it != subproblems.end(); it++)
    {
        if((*it)->getState() == UNASSIGNED)
        {
            (*it)->setState(ASSIGNED_PENDING);
            (*it)->setSIDAssignedTo(worker);
            return (*it);
        }
    }
    return nullptr;
}


void SolutionManager::print()
{
    cout << "Segmenty: " << endl;
    uint32_t radius = K * side;
    for(int i = 0; i < side * side; i++)
    {
        cout << i + 1 << ". id = " << squareSegments[i].getSegmentId() << ", side = " << squareSegments[i].getSide();
        cout << ", xLeft = " << squareSegments[i].getXLeft(radius) << ", xRight = " << squareSegments[i].getXRight(radius);
        cout << ", yBottom = " << squareSegments[i].getYBottom(radius) << ", yTop = " <<squareSegments[i].getYTop(radius);
        cout << ", column = " << squareSegments[i].getColumn() << ", row = " << squareSegments[i].getRow();
        cout << ", pointsHit = " << squareSegments[i].getPointsHit() << endl;
    }
    list<Subproblem*>::iterator it;
    cout << "Podproblemy:" << endl;
    for(it = subproblems.begin(); it != subproblems.end(); it++)
    {
        cout << "SegmentID = " << (*it)->getSegmentId() << ", side = " << (*it)->getSide() << ", points = " << (*it)->getPoints();
        cout << ", state = " << (*it)->getState() << endl;
        if((*it)->getState() == ASSIGNED || (*it)->getState() == ASSIGNED_PENDING)
        {
            cout << "assigned to: " << (*it)->getSIDAssignedTo() << endl;
        }
    }
}
