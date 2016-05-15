// CustomVU.cpp: implementation of the CCustomVU class.
//
//////////////////////////////////////////////////////////////////////

#include "WaveDisplay.h"
#include "math.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------
// CVuMeter
//------------------------------------------------------------------------
CWaveDisplay::CWaveDisplay (const CRect &size, CSmartelectronixDisplay* effect, CBitmap *back, CBitmap *heads, CBitmap *readout)
			 :CControl (size, 0, 0), effect(effect), back(back), heads(heads), readout(readout)
{
	value = oldValue = 0.f;

	srand( (unsigned)time( NULL ) );
	display = rand() % 4;

	if (heads)
		heads->remember();
	if(back)
		back->remember();
	if(readout)
		readout->remember();

	where.x = -1;
	OSDC = 0;
}

//------------------------------------------------------------------------
CWaveDisplay::~CWaveDisplay ()
{
	if(heads)
		heads->forget();
	if(back)
		back->forget();
	if(readout)
		readout->forget();

	if(OSDC != 0)
		delete OSDC;
}

//------------------------------------------------------------------------
void CWaveDisplay::setDirty(const bool val)
{
	CView::setDirty(val);
}

void CWaveDisplay::mouse(CDrawContext *pContext, CPoint &where)
{
	long button = pContext->getMouseButtons();
	
	if(((button & kApple) && (button & kLButton)) || (button & kRButton))
	{
		this->where.x = -1;
	}
	
	while(button & kLButton) //still pushing that button?
	{
		CPoint click;
		pContext->getMouseLocation(click);

		if(size.pointInside(click))
			this->where = click;
		else
			this->where.x = -1;
			
		button = pContext->getMouseButtons();

		doIdleStuff();
	}
}

inline float cf_lin2db(float lin)
{
	if (lin<9e-51) return -1000.f; /* prevent invalid operation */
	return 20.f*log10f(lin);
}

//------------------------------------------------------------------------
void CWaveDisplay::draw (CDrawContext *pContext)
{
	if(OSDC == 0)
		OSDC = new COffscreenContext(getParent(),size.width(),size.height(),kBlackCColor);

	// background, stupid hack, for some reason the rect is offset one pixel??
	CRect R(0,0,size.width(),size.height());
	
	back->draw(OSDC,R,CPoint(size.left,size.top));

	R(615-size.left,240-size.top,615+heads->getWidth()-size.left,240+heads->getHeight()/4-size.top);
	heads->draw(OSDC,R,CPoint(0,(display*heads->getHeight())/4));
	
	// trig-line
	long triggerType = (long)(effect->getParameter(CSmartelectronixDisplay::kTriggerType)*CSmartelectronixDisplay::kNumTriggerTypes + 0.0001);

	if(triggerType == CSmartelectronixDisplay::kTriggerRising || triggerType == CSmartelectronixDisplay::kTriggerFalling)
	{
		long y = 1 + (long)((1.f - effect->getParameter(CSmartelectronixDisplay::kTriggerLevel))*(size.height()-2));

		CColor grey = {229,229,229};
		OSDC->setFrameColor(grey);
		OSDC->moveTo(CPoint(0,y));
		OSDC->lineTo(CPoint(size.width()-1,y));
	}

	// zero-line
	CColor orange = {179,111,56};
	OSDC->setFrameColor(orange);
	OSDC->moveTo(CPoint(0,size.height()/2-1));
	OSDC->lineTo(CPoint(size.width()-1,size.height()/2-1));

	// waveform
	CPoint *points = (effect->getParameter(CSmartelectronixDisplay::kSyncDraw) > 0.5f) ? effect->copy : effect->peaks;
	double counterSpeedInverse = pow(10.f,effect->getParameter(CSmartelectronixDisplay::kTimeWindow)*5.f - 1.5);

	if(counterSpeedInverse < 1.0) //draw interpolated lines!
	{
		CColor blue = {64,148,172};
		OSDC->setFrameColor(blue);

		double phase = counterSpeedInverse;
		double dphase = counterSpeedInverse;

		OSDC->moveTo(points[0]);
			
		for(long i=1;i<size.width()-1;i++)
		{
			long index = (long) phase;
			double alpha = phase - (double)index;

			long xi = i;
			long yi = (long)((1.0-alpha)*points[index<<1].v + alpha*points[(index+1)<<1].v);

			OSDC->lineTo(CPoint(xi,yi));

			phase += dphase;
		}
	}
	else
	{
		CColor grey = {118,118,118};
		OSDC->setFrameColor(grey);
		OSDC->polyLine(points,size.width()*2);
	}

	//TODO clean this mess up...
	if(where.x != -1)
	{
		CPoint whereOffset = where;
		whereOffset.offset(-size.x,-size.y);
		CDrawMode oldMode = OSDC->getDrawMode();
		
		OSDC->setDrawMode(kXorMode);
		
		OSDC->moveTo(CPoint(0,whereOffset.y));
		OSDC->lineTo(CPoint(size.width()-1,whereOffset.y));
		OSDC->moveTo(CPoint(whereOffset.x,0));
		OSDC->lineTo(CPoint(whereOffset.x,size.height()-1));

		float gain = powf(10.f,effect->getParameter(CSmartelectronixDisplay::kAmpWindow)*6.f - 3.f);
		float y = (-2.f * ((float)whereOffset.y + 1.f)/(float)OSC_HEIGHT + 1.f)/gain;
		float x = (float)whereOffset.x * (float)counterSpeedInverse;
		char text[256];

		long lineSize = 10;
		//long border = 2;
		
		//CColor color = {179,111,56,0};
		CColor color = {169,101,46,0};

		OSDC->setFontColor(color);

		OSDC->setFont(kNormalFontSmaller);

		readout->draw(OSDC,CRect(508,8,508+readout->getWidth(),8+readout->getHeight()),CPoint(0,0));
		
		CRect textRect(510,10,650,10+lineSize);
				
		sprintf(text,"y = %.5f", y);
		OSDC->drawString(text,textRect,false,kLeftText);
		textRect.offset(0,lineSize);

		sprintf(text,"y = %.5f dB", cf_lin2db(fabsf(y)));
		OSDC->drawString(text,textRect,false,kLeftText);	
		textRect.offset(0,lineSize*2);

		sprintf(text,"x = %.2f samples", x);
		OSDC->drawString(text,textRect,false,kLeftText);
		textRect.offset(0,lineSize);
		
		sprintf(text,"x = %.5f seconds", x/effect->getSampleRate());
		OSDC->drawString(text,textRect,false,kLeftText);
		textRect.offset(0,lineSize);
		
		sprintf(text,"x = %.5f ms", 1000.f*x/effect->getSampleRate());
		OSDC->drawString(text,textRect,false,kLeftText);
		textRect.offset(0,lineSize);

		sprintf(text,(x != 0) ? "x = %.3f Hz" : "x = infinite Hz", effect->getSampleRate()/x);
		OSDC->drawString(text,textRect,false,kLeftText);
		
		
		OSDC->setDrawMode(oldMode);
	}
		
	OSDC->copyFrom(pContext,size,CPoint(0,0));
}
