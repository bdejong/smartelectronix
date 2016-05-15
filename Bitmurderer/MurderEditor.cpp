// MurderEditor.cpp: implementation of the MurderEditor class.
//
//////////////////////////////////////////////////////////////////////

#include "MurderEditor.h"
#include "vstxsynth.h"

#include <stdlib.h>    
#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static CBitmap *background = 0;

enum
{
	kBackground = 101,
	kKnob1 = 103,
	kKnob2 = 104,
	kSplash = 102,
	kBackWidth = 428,
	kBackHeight = 220
};

MurderEditor::MurderEditor(AudioEffect *effect) : AEffGUIEditor (effect)
{
	frame = 0;

    rect.left   = 0;
    rect.top    = 0;
    rect.right  = kBackWidth;
    rect.bottom = kBackHeight;

	points = new CPoint[18];
	
	points[0](160,7);
	points[1](205,7);
	points[2](250,7);
	points[3](295,7);
	points[4](340,7);
	points[5](160,77);
	points[6](205,77);
	points[7](250,77);
	points[8](295,77);
	points[9](340,77);
	points[10](160,147);
	points[11](205,147);
	points[12](250,147);
	points[13](295,147);
	points[14](340,147);
	points[15](70,75);
	points[16](10,100);
	points[17](70,150);


	buttons = new COnOffButton *[18];
}

MurderEditor::~MurderEditor()
{
	delete points;
	delete buttons;
}

long MurderEditor::open (void *ptr)
{
    AEffGUIEditor::open (ptr);

    // init the background bitmap (a global bitmap for all instance of this plugin)
    if (!background)
        background = new CBitmap (kBackground);
    else
        background->remember ();

	//--CFrame-----------------------------------------------
	CRect size (0, 0, background->getWidth () + 100, background->getHeight ());
	frame = new CFrame (size, ptr, this);
	frame->setBackground (background);
	//background->forget ();

	//knobz
	
	CBitmap *knob1 = new CBitmap (kKnob1);
	CBitmap *knob2 = new CBitmap (kKnob2);

	for(char i=0;i<kBits;i++)
	{
		size (0, 0, knob1->getWidth (), knob1->getHeight () / 2);
		size.offset(points[i].h,points[i].v);
		buttons[i] = new COnOffButton(size, this, i, knob1);
		buttons[i]->setValue(effect->getParameter(i));
		frame->addView(buttons[i]);
	}

	for(;i<18;i++)
	{
		size (0, 0, knob2->getWidth (), knob2->getHeight () / 2);
		size.offset(points[i].h,points[i].v);
		buttons[i] = new COnOffButton(size, this, i, knob2);
		buttons[i]->setValue(effect->getParameter(i));
		frame->addView(buttons[i]);
	}

	knob1->forget();
	knob2->forget();

	//splash

	CBitmap *splashBitmap = new CBitmap (kSplash);
	size (395,165,428,201);
	CRect toDisplay (0, 0, splashBitmap->getWidth (), splashBitmap->getHeight ());
	splash = new CSplashScreen (size, this, 2000, splashBitmap, toDisplay, CPoint(0, 0));
	frame->addView (splash);
	splashBitmap->forget ();

	return true;
}

void MurderEditor::close()
{
	    // don't forget to remove the frame !!
    if (frame)
        delete frame;
    frame = 0;

    // forget background if not anymore used
    if (background)
    {
        if (background->getNbReference () <= 1)
        {
            background->forget ();    
            background = 0;
        }
        else
            background->forget ();    
    }

	for(char i=0;i<18;i++)
		buttons[i] = 0;

	splash = 0;
}

void MurderEditor::setParameter(long index, float value)
{
	if (!frame)
		return;

	if(index < kNumParams)
		buttons[index]->setValue(effect->getParameter(index));

}

void MurderEditor::valueChanged(CDrawContext* context, CControl* control)
{
	if(control->getTag() < kNumParams)
		effect->setParameter(control->getTag(),control->getValue());
	else
		control->update (context);
}
