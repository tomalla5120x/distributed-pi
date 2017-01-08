#ifndef MESSAGE_H
#define MESSAGE_H
#include <stdint.h>

/*
* enum z typem wiadomosci. wybor rozmiaru wymaga c++11
* i w sumie nie wiem czy to cos pomoze - strukturki sa i tak paddowane - 1 + 4 + 8 + 4 + 4 = 21, paddowane do 24
*/
enum MessageType : uint8_t
{						//wielkosci w bajtach dla typow:
	MessageHeartbeat = 0, 	// 1 bo tylko tag 1 bajtowy
	MessageHeartbeatACK = 1,// 1
	MessageInterrupt = 2,	// 1
	MessageHello = 3,		// 5 bo tag + sequence (int, 4)
	MessageACK = 4,			// 5 
	MessageClose = 5,		// 5
	MessageResult = 6,		// 17 bo tag + seq + points (longlong, 8) + segment_id (int, 4)
	MessageWork = 7,		// najbardziej dokokszony, 21
	NoMessage = 0xFE,		// brak wiadomosci
	MessageInvalid = 0xFF // nie powinien byc wyslany, ale 1
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

	Message(): mTag(MessageInvalid) {};

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

#endif
