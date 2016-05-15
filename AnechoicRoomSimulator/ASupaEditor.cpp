// ASupaEditor.cpp: implementation of the ASupaEditor class.
//
//////////////////////////////////////////////////////////////////////

#include "ASupaEditor.h"
#include "DelayExample.hpp"
#include <stdio.h>
#include "math.h"
#include "resource.h"

ASupaEditor::ASupaEditor(AudioEffect *effect):AEffGUIEditor (effect)
{
	sizeKnob = 0;
	sizeDisplay = 0;
	frame = 0;

	// init the size of the plugin
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = 302;
	rect.bottom = 192;
}

ASupaEditor::~ASupaEditor()
{
}

long ASupaEditor::open (void *ptr)
{
	AEffGUIEditor::open(ptr);

	CBitmap *hBackground = new CBitmap(IDB_BACK);
	CBitmap *distknob = new CBitmap(IDB_KNOB);
	CBitmap *greyText = new CBitmap(IDB_TYPE);
	CBitmap *splashBitmap = new CBitmap(IDB_SPLASH);
	
	//init frame
	CRect size(0, 0, hBackground->getWidth (), hBackground->getHeight ());
	frame = new CFrame(size, ptr, this);
	frame->setBackground(hBackground);
	setKnobMode(kLinearMode);

	CColor grey = {118,118,118};
	CColor white = {255,255,255};
	CColor red = {255,0,0};

	long displayWidth = 30;
	long displayHeight = 6;

	char temp[256];
	((CDelayExample *)effect)->getDisplay(CDelayExample::kSize,temp);
	sizeDisplay = new CTextDisplay(CRect(39,159,103,159+displayHeight),greyText,grey);
	sizeDisplay->setText(temp);
	frame->addView(sizeDisplay);

	// distortion
	sizeKnob = new CAnimKnob(CRect(117, 63, 117 + distknob->getWidth(), 63 + distknob->getHeight()/111), this, CDelayExample::kSize, 111, 68, distknob, CPoint(0,0));
	sizeKnob->setDefaultValue(0.5f);
	sizeKnob->setValue(effect->getParameter(CDelayExample::kSize));
	frame->addView(sizeKnob);


	splash = new CSplashScreen(CRect(220,23,275,74), this, 1000, splashBitmap, size, CPoint(0,0));
	frame->addView(splash);

	// forget bitmaps
	hBackground->forget();
	distknob->forget();
	greyText->forget();
	splashBitmap->forget();
	
	return true;
}

void ASupaEditor::close ()
{
	sizeKnob = 0;
	sizeDisplay = 0;
	splash = 0;

	if(frame != 0)
		delete frame;

	frame = 0;
}

void ASupaEditor::setParameter (long index, float value)
{
	if (frame == 0)
		return;

	if(index >= 0 && index < CDelayExample::kNumParams)
	{
		switch(index)
		{
			case CDelayExample::kSize	: if(sizeKnob) sizeKnob->setValue(value); break;
		}

		char temp[256];
		((CDelayExample *)effect)->getDisplay(index,temp);
			
		switch(index)
		{
			case CDelayExample::kSize	: if(sizeDisplay) sizeDisplay->setText(temp); break;
		}
	}

	postUpdate();
}

void ASupaEditor::valueChanged (CDrawContext* context, CControl* control)
{
	long tag = control->getTag();

	if(tag >= 0 && tag < CDelayExample::kNumParams)
	{
		char temp[256];
		((CDelayExample *)effect)->getDisplay(tag,temp);

		switch(tag)
		{
			case CDelayExample::kSize	: if(sizeDisplay) sizeDisplay->setText(temp); break;
		}

		//update control...
		effect->setParameterAutomated(tag, control->getValue ());
	}

	if(context != 0)
		control->update(context);
}


	

