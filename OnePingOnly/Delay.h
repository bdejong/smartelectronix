#pragma once

class CDelay
{
public:
	void setFeed(float feed);
	void KillBuffer();
	void setMaxDelay(int MaxDelay=44100*5);
	float getVal(float in);
	void setDelay(int d);
	CDelay(int MaxDelay);
	virtual ~CDelay();
private:
	float *buffer;
	float feed,feedtmp;
	int index;
	int pos;
	int delay;
	int MaxDelay;
};
