#include <iostream>
#include <cstdint>
#include <list>
#include "segment.h"
#include "subproblem.h"
using namespace std;

#ifndef SOLUTION_MANAGER_H
#define SOLUTION_MANAGER_H

class SolutionManager
{
private:
    uint32_t side; // M
    Segment* squareSegments; //tablica wszystkich segmentow, indeks w tablicy to segmentId - 1
    list<Subproblem*> subproblems; //lista podproblemow
    uint64_t points; //N
    uint32_t digitsAfterDot;
public:
    SolutionManager(uint32_t mSide, uint64_t nPoints, uint32_t digitsAfterDot);
    ~SolutionManager();
    void initialize();
    void markSolved(uint32_t segmentId, uint64_t pointsHit);
    void assign(uint32_t segmentId, SID worker);
    void unassign(uint32_t segmentId);
    bool solved();
    string getResult();
    Subproblem* pop(SID worker);
    void print(); //do debugowania
};

#endif // SOLUTION_MANAGER_H
