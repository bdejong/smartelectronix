// MovingDelay.h: interface for the CMovingDelay class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOVINGDELAY_H__D7D8BCC0_72BC_11D3_9312_444553540000__INCLUDED_)
#define AFX_MOVINGDELAY_H__D7D8BCC0_72BC_11D3_9312_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMovingDelay  
{
public:
	CMovingDelay(long max = 1000000)
	{
		buffer = new float[max];
		index = max-1;
		this->max = max;
		suspend();
	}

	virtual ~CMovingDelay()
	{
		delete buffer;
	};

	inline float GetVal(float in, float delay)
	{
		buffer[index] = in;

		float pointer = (float)index + delay;
		unsigned long outindex = (unsigned long) pointer;
		float alpha = pointer - outindex;
		
		if(outindex >= max)
			outindex -= max;

		unsigned long outindex1 = outindex + 1;
		if(outindex1 >= max)
			outindex1 -= max;
		
		index--;
		if(index >= max)
			index = max - 1;
	
		return (1.f - alpha)*buffer[outindex] + alpha*buffer[outindex1];
	};
	
	void suspend()
	{
		for(unsigned long i=0;i<max;i++)
			buffer[i] = 0;
	};

	unsigned long max;

private:
	float *buffer;
	unsigned long index;
};

#endif // !defined(AFX_MOVINGDELAY_H__D7D8BCC0_72BC_11D3_9312_444553540000__INCLUDED_)
