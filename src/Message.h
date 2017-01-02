#include <stdint.h>

/*
* enum z typem wiadomosci. wybor rozmiaru wymaga c++11
* i w sumie nie wiem czy to cos pomoze - strukturki sa i tak paddowane - 1 + 4 + 8 + 4 + 4 = 21, paddowane do 24
*/
enum MessageType : uint8_t
{
	MessageHeartbeat,
	MessageHeartbeatACK,
	MessageInterrupt,
	MessageHello,
	MessageACK,
	MessageClose,
	MessageResult,
	MessageWork
};

/**
* Klasa Message obejmuje tylko i wylacznie sama wiadomosc, wystawia na swiat tylko gettery i konstruktory dla pozadanych typow wiadomosci
*/
class Message {
public:
	MessageType	getTag() 		{ return mTag; };
	uint32_t	getSequence() 	{ return mSequence; };
	uint64_t	getPoints() 	{ return mPoints; };
	uint64_t	getPointsHit() 	{ return mPointsHit; };
	uint32_t	getSegmentID()	{ return mSegmentID; };
	uint32_t	getSide()		{ return mSide; };

	/**
	* Do uzycia z MessageInterrupt, Heartbeat, HeartbeatACK
	* mozemy dodac opcje sprawdzania typu i rzucania wyjatku? 
	* ale to jest cos co zepsuc moze tylko programista i w sumie kompletnie niepotrzebna robota
	*/
	Message(MessageType tag) : 
			mTag(tag) 
			{};
	/**
	* Do uzycia z MessageHello, ACK, Close
	*/
	Message(MessageType tag, uint32_t sequence) : 
			mTag(tag), mSequence(sequence) 
			{};
	/**
	* Do uzycia z MessageResult
	*/
	Message(MessageType tag, uint32_t sequence, uint32_t segment_id, uint64_t points_hit) :
			mTag(tag), mSequence(sequence), mPointsHit(points_hit), mSegmentID(segment_id) 
			{};
	/**
	* Do uzycia z MessageWork
	*/ 
	Message(MessageType tag, uint32_t sequence, uint32_t segment_id, uint64_t points, uint32_t side) :
			mTag(tag), mSequence(sequence), mPoints(points), mSegmentID(segment_id), mSide(side) 
			{};
			

private:
	/**
	* Stuff do wyslania i nic wiecej
	*/
	MessageType mTag;
	uint32_t	mSequence;
	union {
		uint64_t mPoints;
		uint64_t mPointsHit;
	};
	uint32_t	mSegmentID;
	uint32_t	mSide;
};
