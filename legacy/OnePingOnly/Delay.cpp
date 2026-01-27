// Delay.cpp: implementation of the CDelay class.
//
//////////////////////////////////////////////////////////////////////

#include "Delay.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDelay::CDelay(int MaxDelay)
{
	this->MaxDelay = MaxDelay;
	index=pos=0;

	buffer = new float[MaxDelay];
	KillBuffer();

	this->setFeed(0.f);

	delay = 0;
}

CDelay::~CDelay()
{
	delete buffer;
}

void CDelay::setDelay(int d)
{
	pos = index - d;
	while(pos>MaxDelay)
		pos -= MaxDelay;
	while(pos<0)
		pos += MaxDelay;

	delay = d;
}

float CDelay::getVal(float in)
{
	float output = in + feed*buffer[pos];
	
	buffer[index] = output;
		
	++index;
	if(index==MaxDelay)
		index = 0;
	
	++pos;
	if(pos==MaxDelay)
		pos = 0;

	return output;
}

void CDelay::setMaxDelay(int MaxDelay)
{
	this->MaxDelay = MaxDelay;
	
	delete buffer;
	buffer = new float[MaxDelay];
	KillBuffer();

	index = 0; //reset EVERYTHING!
		
	this->setDelay(delay);
}

void CDelay::KillBuffer()
{
	for(int i=0;i<MaxDelay;i++)
		buffer[i] = 0.f;
}

void CDelay::setFeed(float feed)
{
	this->feed = feed;
	this->feedtmp = 1-feed;
}
