// H2Oeditor.cpp: implementation of the H2Oeditor class.
//
//////////////////////////////////////////////////////////////////////

#include "H2Oeditor.h"
#include "H2Oeffect.h"

#include <stdio.h>
#include <math.h>

#if MAC  
enum
{
	IDB_BACK = 128,
	IDB_KNOB,
	IDB_SATURATE,
};
#else
#include "resource.h"
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

H2Oeditor::H2Oeditor(AudioEffect *effect):AEffGUIEditor (effect)
{
	attack=release=ingain=outgain=amount=0;
	
	hBack = new CBitmap (IDB_BACK);
	hKnob = new CBitmap (IDB_KNOB);
	hSaturate = new CBitmap (IDB_SATURATE);
	hSplash = new CBitmap(IDB_SPLASH);

	text = 0;

	// init the size of the plugin
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = (short)hBack->getWidth ();
	rect.bottom = (short)hBack->getHeight ();
}

H2Oeditor::~H2Oeditor()
{
	if(hBack)
		hBack->forget();
	hBack = 0;

	if(hKnob)
		hKnob->forget();
	hKnob = 0;

	if(hSaturate)
		hSaturate->forget();
	hSaturate=0;

	if(hSplash)
		hSplash->forget();
	hSplash=0;
}

long H2Oeditor::open(void *ptr)
{
	// !!! always call this !!!
	AEffGUIEditor::open (ptr);

	CRect size (0, 0, hBack->getWidth (), hBack->getHeight ());
	frame = new CFrame (size, ptr, this);
	frame->setBackground (hBack);

	CPoint point(0,0);

	int offset = 39;

	size(52,55,52+hKnob->getWidth(),55+hKnob->getHeight()/60);
	ingain = new CAnimKnob(size,this,kPreAmp,60,hKnob->getHeight()/60,hKnob,point);
	ingain->setValue(effect->getParameter(kPreAmp));
	ingain->setTransparency(false);
	frame->addView(ingain);

	size.offset(offset,0);
	attack = new CAnimKnob(size,this,kAttack,60,hKnob->getHeight()/60,hKnob,point);
	attack->setValue(effect->getParameter(kAttack));
	attack->setTransparency(false);
	frame->addView(attack);

	size.offset(offset,0);
	release = new CAnimKnob(size,this,kRelease,60,hKnob->getHeight()/60,hKnob,point);
	release->setValue(effect->getParameter(kRelease));
	release->setTransparency(false);
	frame->addView(release);

	size(168,55,168+hKnob->getWidth(),55+hKnob->getHeight()/60);
	amount = new CAnimKnob(size,this,kAmount,60,hKnob->getHeight()/60,hKnob,point);
	amount->setValue(effect->getParameter(kAmount));
	amount->setTransparency(false);
	frame->addView(amount);

	size.offset(offset,0);
	outgain = new CAnimKnob(size,this,kPostAmp,60,hKnob->getHeight()/60,hKnob,point);
	outgain->setValue(effect->getParameter(kPostAmp));
	outgain->setTransparency(false);
	frame->addView(outgain);

	size(169,94,169+hSaturate->getWidth(),94+hSaturate->getHeight()/2);
	saturate = new COnOffButton(size,this,kSaturate,hSaturate);
	saturate->setValue(effect->getParameter(kSaturate));
	frame->addView(saturate);

	size(58,98,152,114);
	text = new CTextDisplay(size,this,400);
	text->setText("H2O");
	frame->addView(text);

	size(208,94,234,121);
	splash1 = new CSplashScreen(size,this,kSplash1,hSplash,CRect(0,0,hSplash->getWidth(),hSplash->getHeight()),CPoint(0,0));
	frame->addView(splash1);

	size(124,134,271,159);
	splash2 = new CSplashScreen(size,this,kSplash2,hSplash,CRect(0,0,hSplash->getWidth(),hSplash->getHeight()),CPoint(0,0));
	frame->addView(splash2);

	size(15,8,94,35);
	splash3 = new CSplashScreen(size,this,kSplash3,hSplash,CRect(0,0,hSplash->getWidth(),hSplash->getHeight()),CPoint(0,0));
	frame->addView(splash3);

	setKnobMode(kLinearMode);

	return true;
}

void H2Oeditor::close()
{
	delete frame;
	frame = 0;
}

void H2Oeditor::setParameter(long index, float value)
{
	if(!frame)
		return;

	char buffer[50];
	buffer[0] = 0;
	char buffer2[10];
	buffer2[0] = ' ';
	buffer2[1] = 0;

	switch(index)
	{
		case kPreAmp:
			if(ingain)
				ingain->setValue(effect->getParameter(index));
			
			effect->getParameterDisplay(index,buffer);
			effect->getParameterLabel(index,buffer2+1);
			strcat(buffer,buffer2);
			if(text)
				text->setText(buffer);
			break;
		case kAmount:
			if(amount)
				amount->setValue(effect->getParameter(index));

			effect->getParameterDisplay(index,buffer);
			effect->getParameterLabel(index,buffer2+1);
			strcat(buffer,buffer2);
			if(text)
				text->setText(buffer);
			break;
		case kAttack:
			if(attack)
				attack->setValue(effect->getParameter(index));
			
			effect->getParameterDisplay(index,buffer);
			effect->getParameterLabel(index,buffer2+1);
			strcat(buffer,buffer2);
			if(text)
				text->setText(buffer);
			break;
		case kRelease:
			if(release)
				release->setValue(effect->getParameter(index));

			effect->getParameterDisplay(index,buffer);
			effect->getParameterLabel(index,buffer2+1);
			strcat(buffer,buffer2);
			if(text)
				text->setText(buffer);
			break;
		case kPostAmp:
			if(outgain)
				outgain->setValue(effect->getParameter(index));
			
			effect->getParameterDisplay(index,buffer);
			effect->getParameterLabel(index,buffer2+1);
			strcat(buffer,buffer2);
			if(text)
				text->setText(buffer);
			break;
		case kSaturate:
			if(saturate)
				saturate->setValue(effect->getParameter(index));
			
			effect->getParameterDisplay(index,buffer);
			effect->getParameterLabel(index,buffer2+1);
			strcat(buffer,buffer2);
			if(text)
				text->setText(buffer);
			break;
	}

	postUpdate();
}

void H2Oeditor::valueChanged(CDrawContext* context, CControl* control)
{
	long tag = control->getTag ();

	effect->setParameterAutomated(tag, control->getValue ());
	control->update (context);
	
	text->update(context);
}
