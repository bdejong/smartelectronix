#include "CyanideEditor.h"
#include "CyanideEffect.h"
#include "resource.h"

static CBitmap *hBack = 0;

#define nDisp 12
const char *disp[nDisp] = {"Like this? Make a donation!", "Welcome back!","Crunch that beat!","Zap that beat!!","Zap that beat!!","Zap that beat!!","Zap that beat!!","Zap that beat!!","Stop reopening me!","Nooooooo!","Have MERCY!!","*arghl* I'm dead."};

CyanideEditor::CyanideEditor(AudioEffect *effect, CShaper *pShaper) : AEffGUIEditor (effect)
{
	this->pShaper = pShaper;
	
	// init the size of the plugin
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = 380;
	rect.bottom = 350;

	hPreGain = 0;
	hPreFilter = 0;
	hPostGain = 0;
	hPostFilter = 0;
	hPreType = 0;
	hPostType = 0;
	hDryWet = 0;
	hSplashScreen = 0;
	hGuides = 0;
	hReset = 0;
	hOversample = 0;
	hShaper = 0;
	hPreview = 0;
	pShaper = 0;
	hLabel = 0;

	opened = 0;
}

CyanideEditor::~CyanideEditor()
{
}

long CyanideEditor::open (void *ptr)
{
	// !!! always call this !!!
	AEffGUIEditor::open (ptr);

	if(hBack == 0)
		hBack = new CBitmap(BACK);
	else
		hBack->remember();

	//init frame
	CRect size (0, 0, hBack->getWidth (), hBack->getHeight ());
	frame = new CFrame (size, ptr, this);
	frame->setBackground (hBack);
	//hBack->forget();  //don't forget static objects!!

	//knobs	
	CBitmap *hKnobBack = new CBitmap(KNOB_BACK);
	CBitmap *hKnobBack2 = new CBitmap(KNOB_BACK2);
	CBitmap *hKnobHandle = new CBitmap(KNOB_HANDLE);

	CPoint zerozero(0,0);
		
	//pregain:
	size(24,100,24+hKnobBack->getWidth(),100+hKnobBack->getHeight());
	hPreGain = new CKnob(size,this,kPreGain,hKnobBack,hKnobHandle,zerozero);
	hPreGain->setInsetValue(3);
	hPreGain->setValue(effect->getParameter(kPreGain));
	frame->addView(hPreGain);

	//pretype
	size(24,159,24+hKnobBack->getWidth(),159+hKnobBack->getHeight());
	hPreType = new CKnob(size,this,kPreType,hKnobBack2,hKnobHandle,zerozero);
	hPreType->setInsetValue(3);
	hPreType->setValue(effect->getParameter(kPreType));
	frame->addView(hPreType);

	//prefilter
	size(24,222,24+hKnobBack->getWidth(),222+hKnobBack->getHeight());
	hPreFilter = new CKnob(size,this,kPreFilter,hKnobBack,hKnobHandle,zerozero);
	hPreFilter->setInsetValue(3);
	hPreFilter->setValue(effect->getParameter(kPreFilter));
	frame->addView(hPreFilter);

	//postgain:
	size(329,100,329+hKnobBack->getWidth(),100+hKnobBack->getHeight());
	hPostGain = new CKnob(size,this,kPostGain,hKnobBack,hKnobHandle,zerozero);
	hPostGain->setInsetValue(3);
	hPostGain->setValue(effect->getParameter(kPostGain));
	frame->addView(hPostGain);

	//posttype
	size(329,159,329+hKnobBack->getWidth(),159+hKnobBack->getHeight());
	hPostType = new CKnob(size,this,kPostType,hKnobBack2,hKnobHandle,zerozero);
	hPostType->setInsetValue(3);
	hPostType->setValue(effect->getParameter(kPostType));
	frame->addView(hPostType);

	//postfilter
	size(329,222,329+hKnobBack->getWidth(),222+hKnobBack->getHeight());
	hPostFilter = new CKnob(size,this,kPostFilter,hKnobBack,hKnobHandle,zerozero);
	hPostFilter->setInsetValue(3);
	hPostFilter->setValue(effect->getParameter(kPostFilter));
	frame->addView(hPostFilter);

	//Dry Wet
	size(329,306,329+hKnobBack->getWidth(),306+hKnobBack->getHeight());
	hDryWet = new CKnob(size,this,kDryWet,hKnobBack,hKnobHandle,zerozero);
	hDryWet->setInsetValue(3);
	hDryWet->setValue(effect->getParameter(kDryWet));
	frame->addView(hDryWet);

	hKnobBack->forget();
	hKnobBack2->forget();
	hKnobHandle->forget();
	
	//oversampling switch
	CBitmap *hSwitchBack = new CBitmap(SWITCH_BACK);
	size(14,291,14+hSwitchBack->getWidth(),291+hSwitchBack->getHeight()/2);
	hOversample = new COnOffButton(size,this,kOverSample,hSwitchBack);
	hOversample->setValue(effect->getParameter(kOverSample));
	frame->addView(hOversample);
	hSwitchBack->forget();

	//preview
	size(80,284,302,344);
	hPreview = new CPreview(size,this,kPreview,hBack,pShaper);
	hPreview->setValue(0.f);
	frame->addView(hPreview);
	pShaper->SetPointerToPreview(hPreview);

	//shaper
	CBitmap *hShaperHandle = new CBitmap(SHAPER_HANDLE);
	size(80,45,302,267);
	hShaper = new CShaperView(size,this,kShaper,hBack,hShaperHandle,pShaper);
	frame->addView(hShaper);
	pShaper->SetPointerToView(hShaper);
	hShaperHandle->forget();

	//guides-knob
	CBitmap *hGuidesBack = new CBitmap(GUIDES);
	size(292,270,292+hGuidesBack->getWidth(),270+hGuidesBack->getHeight()/2);
	hGuides = new COnOffButton(size,this,kGuides,hGuidesBack);
	hGuides->setValue(0.f);
	frame->addView(hGuides);
	hGuidesBack->forget();

	//reset-knob
	CBitmap *hResetBack = new CBitmap(RESET);
	size(278,270,278+hResetBack->getWidth(),270+hResetBack->getHeight()/2);
	hReset = new CKickButton(size,this,kReset,hResetBack->getHeight()/2,hResetBack,zerozero);
	hReset->setValue(0.f);
	frame->addView(hReset);
	hResetBack->forget();

	//SplashScreen
	CBitmap *hSplash = new CBitmap(SPLASH);
	size(0,0,hBack->getWidth(),45); //where to hit
	CRect S(0,26,0+hSplash->getWidth(),26+hSplash->getHeight()); //where to display
	hSplashScreen = new CSplashScreen(size,this,kSplash,hSplash,S,zerozero);
	frame->addView(hSplashScreen);
	hSplash->forget();

	//label
	size(237,25,377,41);
	hLabel = new CLabel(size,disp[(opened++) % nDisp]);
	if(hLabel)
	{
		hLabel->setFont(kNormalFontSmall);
		hLabel->setFontColor(kBlackCColor);
		hLabel->setBackColor(kWhiteCColor);
		frame->addView(hLabel);
	}

	setKnobMode(kLinearMode);

	return true;
}

void CyanideEditor::close()
{
	//kill frame
	if(frame)
		delete frame;

	frame = 0;
	
	// kill the static back
	if(hBack != 0)
	{
		if(hBack->getNbReference() <= 1)
		{
			hBack->forget();
			hBack = 0;
		}
		else
			hBack->forget();
	}

	hPreGain = 0;
	hPreFilter = 0;
	hPostGain = 0;
	hPostFilter = 0;
	hPreType = 0;
	hPostType = 0;
	hDryWet = 0;
	hSplashScreen = 0;
	hGuides = 0;
	hReset = 0;
	hOversample = 0;
	hShaper = 0;
	hPreview = 0;
	hLabel = 0;
	
	//not doing this creates a hard-to-find error :)
	pShaper->SetPointerToView(0);
	pShaper->SetPointerToPreview(0);
}

void CyanideEditor::setParameter (long index, float value)
{
	if (!frame)
		return;

	switch(index)
	{
		case kShaper:
			{
				if(hPreview)
				{
					hPreview->setValue(0.f);
					hPreview->setDirty();
				}
				if(hShaper)
				{
					hShaper->setValue(0.f);
					hShaper->setDirty();
				}
				break;
			};
		case kPreGain:
			{
				if(hPreGain)
					hPreGain->setValue(effect->getParameter(index));
				break;
			};
		case kPreFilter:
			{
				if(hPreFilter)
					hPreFilter->setValue(effect->getParameter(index));
				break;
			};
		case kPostGain:
			{
				if(hPostGain)
					hPostGain->setValue(effect->getParameter(index));
				break;
			};
		case kPostFilter:
			{
				if(hPostFilter)
					hPostFilter->setValue(effect->getParameter(index));
				break;
			};
		case kPreType:
			{
				if(hPreType)
					hPreType->setValue(effect->getParameter(index));
				break;
			};
		case kPostType:
			{
				if(hPostType)
					hPostType->setValue(effect->getParameter(index));
				break;
			};
		case kDryWet:
			{
				if(hDryWet)
					hDryWet->setValue(effect->getParameter(index));
				break;
			};
		case kOverSample:
			{
				if(hOversample)
					hOversample->setValue(effect->getParameter(index));
				break;
			};
	}

	setDisplayText(index);
	
	postUpdate();
}

void CyanideEditor::valueChanged (CDrawContext* context, CControl* control)
{
	long tag = control->getTag();

	switch(control->getTag())
	{
		case kGuides:
			{
				if(hShaper)
				{
					hShaper->setGuides(control->getValue() > 0.5f);
					hShaper->update(context);
				}
				break;
			}
		case kReset:
			{
				if(control->getValue() > 0.5f)
				{
					if(hShaper)
					{
						hShaper->reset();
						hShaper->update(context);
					}
					if(hPreview)
					{
						hPreview->setDirty();
						hPreview->update(context);
					}
					effect->setParameterAutomated(kShaper,0.f);
				}
				break;
			}
		case kShaper:
			{
				if(hPreview)
				{
					hPreview->setDirty();
					hPreview->update(context);
				}
				effect->setParameterAutomated(kShaper,0.f);
				break;
			}
		case kPreFilter:
		case kPostGain:
		case kPostFilter:
		case kPreGain:
		case kPreType:
		case kPostType:
		case kOverSample:
		case kDryWet:
			effect->setParameterAutomated(tag, control->getValue ());
			break;
	}

	control->update(context);
}

void CyanideEditor::setDisplayText(long index)
{
	if(!hLabel)
		return;

	char buffer[256];
	buffer[0] = 0;

	switch(index)
	{
		case kPreGain:
		case kPreFilter:
		case kPostGain:
		case kPostFilter:
		case kPreType:
		case kPostType:
		case kDryWet:
		case kOverSample:
			{
				effect->getParameterDisplay(index,buffer);
				strcpy(&buffer[strlen(buffer)]," ");
				effect->getParameterLabel(index,&buffer[strlen(buffer)]);
				hLabel->setLabel(buffer);
				break;
			}
		case kShaper:
			{
				float x,y;
				if(hShaper->getSelectedXY(x,y))
				{
					effect->float2string(x,buffer);
					strcpy(&buffer[strlen(buffer)]," , ");
					effect->float2string(y,&buffer[strlen(buffer)]);
					hLabel->setLabel(buffer);
				}
				else
					hLabel->setLabel("");
				break;
			}
		default:
			hLabel->setLabel("");
	}
}
