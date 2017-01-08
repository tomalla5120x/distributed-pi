#include "segment.h"
#include <stdexcept>
using namespace std;

bool isPointInsideCircle(double x, double y, uint32_t radius)
{
    return x * x + y * y <= radius * radius;
}

Segment::Segment()
{
    segmentId = 0;
    side = 0;
    pointsHit = 0;
}

void Segment::setSegmentId(uint32_t id)
{
    segmentId = id;
}

uint32_t Segment::getSegmentId()
{
    return segmentId;
}

uint64_t Segment::getPointsHit()
{
    return pointsHit;
}

void Segment::setPointsHit(uint64_t pHit)
{
    pointsHit = pHit;
}

void Segment::setSide(uint32_t s)
{
    if(s == 0 || s > MAX_SIDE_VALUE)
    {
        throw runtime_error("Side value must be between 0 and MAX_VALUE");
    }
    side = s;
}

uint32_t Segment::getSide()
{
    return side;
}

uint32_t Segment::getColumn()
{
    if(side == 0)
    {
        throw runtime_error("Side value is uninitialized");
    }
    if(segmentId % side == 0)
    {
        return side;
    } else {
        return segmentId % side;
    }
}

uint32_t Segment::getRow()
{
    return ((segmentId - getColumn()) / side) + 1;
}

uint32_t Segment::getXLeft()
{
    uint32_t radius = K * side;
    return (getColumn() - 1) * radius / side;
}

uint32_t Segment::getXRight()
{
    uint32_t radius = K * side;
    return getColumn() * radius / side;
}

uint32_t Segment::getYBottom()
{
    uint32_t radius = K * side;
    return (getRow() - 1) * radius / side;
}

uint32_t Segment::getYTop()
{
    uint32_t radius = K * side;
    return getRow() * radius / side;
}

bool Segment::isWholeInsideCircle()
{
    uint32_t radius = K * side;
    return isPointInsideCircle(getXRight(), getYTop(), radius);
}

bool Segment::isWholeOutsideCircle()
{
    uint32_t radius = K * side;
    return !isPointInsideCircle(getXLeft(), getYBottom(), radius);
}

bool Segment::isResultKnown()
{
    //stosunek radius / side powinien byc rowny K
    return  isWholeInsideCircle() || isWholeOutsideCircle();
}
