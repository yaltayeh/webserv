#include "EventData.hpp"

EventData::EventData()
	: isEOF(false), isWrite(false), isRead(false),
	  isEnableReadEvent(false), isEnableWriteEvent(false)
{
}

EventData::~EventData()
{
}
