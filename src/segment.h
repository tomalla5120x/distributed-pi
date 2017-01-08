#include <cstdint>
#ifndef SEGMENT_H
#define SEGMENT_H

const uint32_t K = 3; //wielokrotnosc side, ktora zostanie przyjeta jako promien
const uint32_t MAX_SIDE_VALUE = 10000; //potem brakuje pamieci na tablice dynamiczna, a dla ok 30 000 wynik obliczen jest kiepskiej jakosci

bool isPointInsideCircle(double x, double y, uint32_t radius);

/*
    Segment znajduje sie w I cwiartce ukladu wspolrzednych, segmentId pozwala okreslic wspolrzedne wszstkich wierzcholkow segmentu.
    Segmenty numerowane sa od 1 do side^2, kolejno od lewej do prawej, od dolu do gory.
    Wartosci na osi Y rosna ku gorze, jak w matematyce.
*/

class Segment
{
private:
    uint32_t segmentId;
    uint32_t side; // M
    uint64_t pointsHit; //W
public:
    Segment();
    void setSegmentId(uint32_t id);
    uint32_t getSegmentId();
    uint64_t getPointsHit();
    void setPointsHit(uint64_t pHit);
    void setSide(uint32_t s);
    uint32_t getSide();
    //numer kolumny z przedzialu <1, side> (od lewej do prawej)
    uint32_t getColumn();
    //numer wiersza z przedzialu <1, side> (od dolu do gory)
    uint32_t getRow();
    //wspolrzedne wierzcholkow segmentu
    uint32_t getXLeft();
    uint32_t getXRight();
    uint32_t getYBottom();
    uint32_t getYTop();
    bool isWholeInsideCircle();
    bool isWholeOutsideCircle();
    bool isResultKnown();
};


#endif // SEGMENT_H
