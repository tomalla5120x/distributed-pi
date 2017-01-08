#include <iostream>
#include <cstdint>
#include <list>

#include "server_manager.h"
#include "segment.h"
#include "subproblem.h"

#ifndef SOLUTION_MANAGER_H
#define SOLUTION_MANAGER_H

class SolutionManager
{
private:
    uint32_t side; // M
    Segment* squareSegments; //tablica wszystkich segmentow, indeks w tablicy to segmentId - 1
    std::list<Subproblem*> subproblems; //lista podproblemow
    uint64_t points; //N
    uint32_t digitsAfterDot;
    std::string decimalExpansion(uint64_t a, uint64_t b, uint32_t digits); // obliczanie rozwiniecia dziesietnego zgodnie z zadana liczby cyfr po przecinku
    	
	SolutionManager(SolutionManager const&);
	SolutionManager& operator=(SolutionManager const&);
	~SolutionManager();
    
    SolutionManager(uint32_t mSide, uint64_t nPoints, uint32_t digitsAfterDot);
    
    static bool initialized;
    static uint32_t initSide, initDigits;
    static uint64_t initPoints;
public:
	
	static SolutionManager& getInstance();
	static void configure(uint32_t mSide, uint64_t nPoints, uint32_t digitsAfterDot);
	
    void initialize();
    void markSolved(uint32_t segmentId, uint64_t pointsHit);
    void assign(uint32_t segmentId, SID worker);
    void unassign(uint32_t segmentId);
    bool solved();
    std::string getResult(uint64_t* a, uint64_t* b);
    Subproblem* pop(SID worker);
    void print(); //do debugowania
};

#endif // SOLUTION_MANAGER_H
