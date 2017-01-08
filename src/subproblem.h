#include <cstdint>
#ifndef SUBPROBLEM_H
#define SUBPROBLEM_H

enum SubproblemState {UNASSIGNED, ASSIGNED_PENDING, ASSIGNED, SOLVED};

//typedef uint32_t IPAddress;
//typedef uint16_t Port;
//typedef struct {
//	IPAddress ip;
//	Port port;
//} SID;

class Subproblem
{
private:
    uint32_t segmentId;
    uint32_t side; //M - liczba kolumn i wierszy, na ktore bedzie podzielony kwadrat
    uint64_t points; //N - liczba punktow do wylosowania w segmencie (w calym podproblemie bedzie ich points * side^2)
    SubproblemState state;
    SID assignedTo;
public:
    Subproblem(uint32_t id, uint32_t mSide, uint64_t nPoints);
    void resetSIDAssignedTo(); //do weryfikacji przez Tomka, ale jakiœ reset by siê przyda³
    uint32_t getSegmentId();
    uint32_t getSide();
    uint64_t getPoints();
    SubproblemState getState();
    SID getSIDAssignedTo();
    void setState(SubproblemState newState);
    void setSIDAssignedTo(SID worker);
};


#endif // SUBPROBLEM_H
