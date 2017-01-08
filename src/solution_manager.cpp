#include "solution_manager.h"
#include <stdexcept>
#include <sstream>
using namespace std;

SolutionManager::SolutionManager(uint32_t mSide, uint64_t nPoints, uint32_t digitsAfterDot)
{
    if(mSide == 0 || mSide > MAX_SIDE_VALUE)
    {
        throw runtime_error("Side value must be between 0 and MAX_VALUE");
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
    uint64_t arraySize = side * side;
    cout << arraySize << endl;
    squareSegments = new Segment[arraySize];
    this->digitsAfterDot = digitsAfterDot;
}

SolutionManager::~SolutionManager()
{
    delete [] squareSegments;
    list<Subproblem*>::iterator it;
    for(it = subproblems.begin(); it != subproblems.end(); it++)
    {
        if(*it != nullptr)
        {
            delete (*it);
            (*it) = nullptr;
        }
    }
}

void SolutionManager::initialize()
{
    bool isResultKnown = false;
    uint32_t segmentId = 0;
    for(uint32_t i = 0; i < side * side; i++)
    {
        segmentId = i + 1;
        squareSegments[i].setSegmentId(segmentId);
        squareSegments[i].setSide(side);
        isResultKnown = squareSegments[i].isResultKnown();
        if(isResultKnown)
        {
            if(squareSegments[i].isWholeInsideCircle())
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

string SolutionManager::decimalExpansion(uint64_t a, uint64_t b, uint32_t digits)
{
    stringstream ss;

    ss << (a / b);

    if(digits == 0)
        return ss.str();

    ss << '.';
    uint32_t currentDigit = 0;

    uint64_t remainder = a % b;

    while(currentDigit != digits)
    {
        ++currentDigit;

        remainder *= 10;

        uint64_t newDigit = remainder / b;
        ss << newDigit;

        remainder = remainder - newDigit * b;
    }

    return ss.str();
}

string SolutionManager::getResult()
{
    uint64_t chosenPoints = points * side * side; //suma wylosowanych punktow we wszystkich segmentach
    uint64_t pointsHitSum = 0;
    for(uint32_t i = 0; i < side * side; i++)
    {
        pointsHitSum += squareSegments[i].getPointsHit();
    }
    uint64_t dividend = 4 * pointsHitSum; // dzielna
    return decimalExpansion(dividend, chosenPoints, digitsAfterDot);
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
    for(uint32_t i = 0; i < side * side; i++)
    {
        cout << i + 1 << ". id = " << squareSegments[i].getSegmentId() << ", side = " << squareSegments[i].getSide();
        cout << ", xLeft = " << squareSegments[i].getXLeft() << ", xRight = " << squareSegments[i].getXRight();
        cout << ", yBottom = " << squareSegments[i].getYBottom() << ", yTop = " <<squareSegments[i].getYTop();
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
            cout << "assigned to: " << (*it)->getSIDAssignedTo().ip << ":" << (*it)->getSIDAssignedTo().port << endl;
        }
    }
}
