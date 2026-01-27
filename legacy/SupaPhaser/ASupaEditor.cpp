// ASupaEditor.cpp: implementation of the ASupaEditor class.
//
//////////////////////////////////////////////////////////////////////

#include "ASupaEditor.h"
#include "ASupaPhaser.h"
#include <stdio.h>
#include "math.h"

ASupaEditor::ASupaEditor(AudioEffect *effect):AEffGUIEditor (effect)
{
	attack = 0;
	release = 0;
	envMin = 0;
	envMax = 0;
	freq = 0;
	stereo = 0;
	lfoMin = 0;
	lfoMax = 0;
	feedback = 0;
	stages = 0;
	distort = 0;
	mixture = 0;
	attackDisplay = 0;
	releaseDisplay = 0;
	envMinDisplay = 0;
	envMaxDisplay = 0;
	freqDisplay = 0;
	stereoDisplay = 0;
	lfoMinDisplay = 0;
	lfoMaxDisplay = 0;
	feedbackDisplay = 0;
	stagesDisplay = 0;
	mixtureDisplay1 = 0;
	mixtureDisplay2 = 0;
	dryWetDisplay = 0;
	gainDisplay = 0;
	dryWet = 0;
	gain = 0;
	extend = 0;
	frame = 0;
	splashScreen = 0;

	// init the size of the plugin
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = 600;
	rect.bottom = 212;
}

ASupaEditor::~ASupaEditor()
{
}

bool ASupaEditor::open(void *ptr)
{
	CBitmap *hBackground = new CBitmap("base.png");
	CBitmap *heads = new CBitmap("heads.png");
	CBitmap *blueknob = new CBitmap("blue_knob1_4.png");
	CBitmap *distknob = new CBitmap("bigknob01rotated_cropped2.png");
	CBitmap *smallknob = new CBitmap("small_knob02.png");
	CBitmap *greyText = new CBitmap("type_grey-back_white.png");
	CBitmap *whiteText = new CBitmap("type_white-back.png");
	CBitmap *orangeText = new CBitmap("type_orange-back.png");
	CBitmap *onOff = new CBitmap("extend_on_off.png");
	CBitmap *myFaderHandlePixmap = new CBitmap("slider.png");
	CBitmap *splash = new CBitmap("splash.png");
	
	//init frame
	CRect size(0, 0, hBackground->getWidth (), hBackground->getHeight ());
	frame = new CFrame(size, this);
    frame->open(ptr);
	frame->setBackground(hBackground);
	setKnobMode(kLinearMode);

	CColor grey = {118,118,118};
	CColor white = {255,255,255};
	CColor red = {255,0,0};

	long displayWidth = 30;
	long displayHeight = 6;

	// upper row
	attack = new CAnimKnob(CRect(106, 43, 106 + blueknob->getWidth(), 43 + blueknob->getHeight()/75), this, ASupaPhaser::kAttack, 75, 33, blueknob, CPoint(0,0));
	attack->setDefaultValue(0.f);
	attack->setValue(effect->getParameter(ASupaPhaser::kAttack));
	frame->addView(attack);

	((ASupaPhaser *)effect)->getDisplay(ASupaPhaser::kAttack,temp);
	attackDisplay = new CTextDisplay(CRect(102,89,129,89+displayHeight),greyText,grey);
	attackDisplay->setText(temp);
	frame->addView(attackDisplay);

	release = new CAnimKnob(CRect(162, 43, 162 + blueknob->getWidth(), 43 + blueknob->getHeight()/75), this, ASupaPhaser::kRelease, 75, 33, blueknob, CPoint(0,0));
	release->setDefaultValue(0.f);
	release->setValue(effect->getParameter(ASupaPhaser::kRelease));
	frame->addView(release);

	((ASupaPhaser *)effect)->getDisplay(ASupaPhaser::kRelease,temp);
	releaseDisplay = new CTextDisplay(CRect(161,89,188,89+displayHeight),greyText,grey);
	releaseDisplay->setText(temp);
	frame->addView(releaseDisplay);

	envMin = new CAnimKnob(CRect(219, 43, 219 + blueknob->getWidth(), 43 + blueknob->getHeight()/75), this, ASupaPhaser::kMinEnv, 75, 33, blueknob, CPoint(0,0));
	envMin->setDefaultValue(0.25f);
	envMin->setValue(effect->getParameter(ASupaPhaser::kMinEnv));
	frame->addView(envMin);

	((ASupaPhaser *)effect)->getDisplay(ASupaPhaser::kMinEnv,temp);
	envMinDisplay = new CTextDisplay(CRect(224,89,221+displayWidth,89+displayHeight),greyText,grey);
	envMinDisplay->setText(temp);
	frame->addView(envMinDisplay);

	envMax = new CAnimKnob(CRect(276, 43, 276 + blueknob->getWidth(), 43 + blueknob->getHeight()/75), this, ASupaPhaser::kMaxEnv, 75, 33, blueknob, CPoint(0,0));
	envMax->setDefaultValue(1.f);
	envMax->setValue(effect->getParameter(ASupaPhaser::kMaxEnv));
	frame->addView(envMax);

	((ASupaPhaser *)effect)->getDisplay(ASupaPhaser::kMaxEnv,temp);
	envMaxDisplay = new CTextDisplay(CRect(280,89,277+displayWidth,89+displayHeight),greyText,grey);
	envMaxDisplay->setText(temp);
	frame->addView(envMaxDisplay);

	// upper row
	freq = new CAnimKnob(CRect(106, 135, 106 + blueknob->getWidth(), 135 + blueknob->getHeight()/75), this, ASupaPhaser::kFreq, 75, 33, blueknob, CPoint(0,0));
	freq->setDefaultValue(0.f);
	freq->setValue(effect->getParameter(ASupaPhaser::kFreq));
	frame->addView(freq);

	((ASupaPhaser *)effect)->getDisplay(ASupaPhaser::kFreq,temp);
	freqDisplay = new CTextDisplay(CRect(102,119,130,119+displayHeight),greyText,grey);
	freqDisplay->setText(temp);
	frame->addView(freqDisplay);

	stereo = new CAnimKnob(CRect(162, 135, 162 + blueknob->getWidth(), 135 + blueknob->getHeight()/75), this, ASupaPhaser::kStereo, 75, 33, blueknob, CPoint(0,0));
	stereo->setDefaultValue(1.f);
	stereo->setValue(effect->getParameter(ASupaPhaser::kStereo));
	frame->addView(stereo);

	((ASupaPhaser *)effect)->getDisplay(ASupaPhaser::kStereo,temp);
	stereoDisplay = new CTextDisplay(CRect(167,119,163+displayWidth,119+displayHeight),greyText,grey);
	stereoDisplay->setText(temp);
	frame->addView(stereoDisplay);

	lfoMin = new CAnimKnob(CRect(219, 135, 219 + blueknob->getWidth(), 135 + blueknob->getHeight()/75), this, ASupaPhaser::kMinFreq, 75, 33, blueknob, CPoint(0,0));
	lfoMin->setDefaultValue(0.f);
	lfoMin->setValue(effect->getParameter(ASupaPhaser::kMinFreq));
	frame->addView(lfoMin);

	((ASupaPhaser *)effect)->getDisplay(ASupaPhaser::kMinFreq,temp);
	lfoMinDisplay = new CTextDisplay(CRect(224,119,221+displayWidth,119+displayHeight),greyText,grey);
	lfoMinDisplay->setText(temp);
	frame->addView(lfoMinDisplay);

	lfoMax = new CAnimKnob(CRect(276, 135, 276 + blueknob->getWidth(), 135 + blueknob->getHeight()/75), this, ASupaPhaser::kMaxFreq, 75, 33, blueknob, CPoint(0,0));
	lfoMax->setDefaultValue(1.f);
	lfoMax->setValue(effect->getParameter(ASupaPhaser::kMaxFreq));
	frame->addView(lfoMax);

	((ASupaPhaser *)effect)->getDisplay(ASupaPhaser::kMaxFreq,temp);
	lfoMaxDisplay = new CTextDisplay(CRect(280,119,277+displayWidth,119+displayHeight),greyText,grey);
	lfoMaxDisplay->setText(temp);
	frame->addView(lfoMaxDisplay);

	// feedback
	feedback = new CAnimKnob(CRect(452, 48, 452 + blueknob->getWidth(), 48 + blueknob->getHeight()/75), this, ASupaPhaser::kFeed, 75, 33, blueknob, CPoint(0,0));
	feedback->setDefaultValue(0.f);
	feedback->setValue(effect->getParameter(ASupaPhaser::kFeed));
	frame->addView(feedback);

	((ASupaPhaser *)effect)->getDisplay(ASupaPhaser::kFeed,temp);
	feedbackDisplay = new CTextDisplay(CRect(461,94,454+displayWidth,94+displayHeight),greyText,grey);
	feedbackDisplay->setText(temp);
	frame->addView(feedbackDisplay);

	// stages
	stages = new CAnimKnob(CRect(452, 131, 452 + blueknob->getWidth(), 131 + blueknob->getHeight()/75), this, ASupaPhaser::knStages, 75, 33, blueknob, CPoint(0,0));
	stages->setDefaultValue(0.5f);
	stages->setValue(effect->getParameter(ASupaPhaser::knStages));
	frame->addView(stages);

	((ASupaPhaser *)effect)->getDisplay(ASupaPhaser::knStages,temp);
	stagesDisplay = new CTextDisplay(CRect(465,115,454+displayWidth,115+displayHeight),greyText,grey);
	stagesDisplay->setText(temp);
	frame->addView(stagesDisplay);

	// distortion
	distort = new CAnimKnob(CRect(349, 72, 349 + distknob->getWidth(), 72 + distknob->getHeight()/111), this, ASupaPhaser::kDistort, 111, 68, distknob, CPoint(0,0));
	distort->setDefaultValue(0.5f);
	distort->setValue(effect->getParameter(ASupaPhaser::kDistort));
	frame->addView(distort);

	// mixture
	mixture = new CAnimKnob(CRect(28, 87, 28 + smallknob->getWidth(), 87 + smallknob->getHeight()/62), this, ASupaPhaser::kMixture, 62, 38, smallknob, CPoint(0,0));
	mixture->setDefaultValue(0.f);
	mixture->setValue(effect->getParameter(ASupaPhaser::kMixture));
	mixture->setInverseBitmap(true);
	frame->addView(mixture);

	long mix = (long) (100 * effect->getParameter(ASupaPhaser::kMixture) );

    vstint2string(mix,temp);
	effect->getParameterLabel(ASupaPhaser::kMixture,&temp[strlen(temp)]);
	mixtureDisplay1 = new CTextDisplay(CRect(30,68,55,74),whiteText,white);
	mixtureDisplay1->setText(temp);
	frame->addView(mixtureDisplay1);

    vstint2string(100 - mix,temp);
	effect->getParameterLabel(ASupaPhaser::kMixture,&temp[strlen(temp)]);
	mixtureDisplay2 = new CTextDisplay(CRect(30,139,55,145),whiteText,white);
	mixtureDisplay2->setText(temp);
	frame->addView(mixtureDisplay2);

	//extend
	extend = new COnOffButton(CRect(326, 155, 326+onOff->getWidth(), 155+onOff->getHeight()/2),this,ASupaPhaser::kExtend,onOff);
	extend->setValue(effect->getParameter(ASupaPhaser::kExtend));
	frame->addView(extend);
	
	// dry/wet slider
	
	CRect size1(
		525,
		45,
		525 + myFaderHandlePixmap->getWidth()+1,
		160 + myFaderHandlePixmap->getHeight()
		);
	int minPos = size1.top;
	int maxPos = size1.bottom - myFaderHandlePixmap->getHeight(); // + myFaderHandlePixmap->getHeight();
	
	CPoint offset (size1.left,size1.top);
	dryWet = new CVerticalSlider (size1,this,ASupaPhaser::kDryWet,minPos,maxPos,myFaderHandlePixmap,hBackground,offset,kBottom);
	dryWet->setValue(effect->getParameter(ASupaPhaser::kDryWet));
	dryWet->setOffsetHandle(CPoint(1,0));
	frame->addView (dryWet);

	// gain slider
	size1.offset(28,0);
	
	offset(size1.left,size1.top);
	gain = new CVerticalSlider (size1,this,ASupaPhaser::kGain,minPos,maxPos,myFaderHandlePixmap,hBackground,offset,kBottom);
	gain->setValue(effect->getParameter(ASupaPhaser::kGain));
	gain->setOffsetHandle(CPoint(2,0));
	gain->setDefaultValue(0.76f);
	frame->addView (gain);

	((ASupaPhaser *)effect)->getDisplay(ASupaPhaser::kGain,temp);
	gainDisplay = new CTextDisplay(CRect(549,174,572,179),whiteText,white);
	gainDisplay->setText(temp);
	gainDisplay->setAlignMiddle();
	frame->addView(gainDisplay);

	//headButton = new CMultiStateButton(CRect(323,23,232+heads->getWidth(),23+heads->getHeight()/4),this,1000,heads,4,heads->getHeight()/4);
	headButton = new CMultiStateButton(CRect(319,22,319+heads->getWidth(),22+heads->getHeight()/4),this,ASupaPhaser::kInvert,heads,4,heads->getHeight()/4);
	headButton->setValue(effect->getParameter(ASupaPhaser::kInvert));
	frame->addView(headButton);

	//CCustomSplashScreen (const CRect &size, CControlListener *listener, long tag, CBitmap *background, CRect &toDisplay, CPoint &offset);
	splashScreen = new CCustomSplashScreen(CRect(396,6,516,29),this,1000,splash,CRect(201,44,201+splash->getWidth(),44+splash->getHeight()),CPoint(0,0));
	frame->addView(splashScreen);

	// forget bitmaps
	myFaderHandlePixmap->forget();
	heads->forget();
	hBackground->forget();
	blueknob->forget();
	distknob->forget();
	smallknob->forget();
	greyText->forget();
	whiteText->forget();
	orangeText->forget();
	onOff->forget();
	splash->forget();

	return true;
}

void ASupaEditor::close ()
{
	attack = 0;
	release = 0;
	envMin = 0;
	envMax = 0;
	freq = 0;
	stereo = 0;
	lfoMin = 0;
	lfoMax = 0;
	feedback = 0;
	stages = 0;
	distort = 0;
	mixture = 0;
	attackDisplay = 0;
	releaseDisplay = 0;
	envMinDisplay = 0;
	envMaxDisplay = 0;
	freqDisplay = 0;
	stereoDisplay = 0;
	lfoMinDisplay = 0;
	lfoMaxDisplay = 0;
	feedbackDisplay = 0;
	stagesDisplay = 0;
	mixtureDisplay1 = 0;
	mixtureDisplay2 = 0;
	dryWetDisplay = 0;
	gainDisplay = 0;
	dryWet = 0;
	gain = 0;
	extend = 0;
	headButton = 0;
	splashScreen = 0;

    if (frame != 0) {
        CFrame *oldFrame = frame;
        frame = 0;
        oldFrame->forget();
    }
}

void ASupaEditor::setParameter(VstInt32 index, float value)
{
	if (frame == 0)
		return;

	if(index >= 0 && index < ASupaPhaser::kNumParams)
	{
		switch(index)
		{
			case ASupaPhaser::kAttack	: if(attack) attack->setValue(value); break;
			case ASupaPhaser::kRelease	: if(release) release->setValue(value); break;
			case ASupaPhaser::kMaxEnv	: if(envMax) envMax->setValue(value); break;
			case ASupaPhaser::kMinEnv	: if(envMin) envMin->setValue(value); break;
			case ASupaPhaser::kFreq		: if(freq) freq->setValue(value); break;
			case ASupaPhaser::kMinFreq	: if(lfoMin) lfoMin->setValue(value); break;
			case ASupaPhaser::kMaxFreq	: if(lfoMax) lfoMax->setValue(value); break;
			case ASupaPhaser::kStereo	: if(stereo) stereo->setValue(value); break;
			case ASupaPhaser::kDistort	: if(distort) distort->setValue(value); break;
			case ASupaPhaser::kMixture	: if(mixture) mixture->setValue(value); break;
			case ASupaPhaser::kExtend	: if(extend) extend->setValue(value); break;
			case ASupaPhaser::kFeed		: if(feedback) feedback->setValue(value); break;
			case ASupaPhaser::knStages	: if(stages) stages->setValue(value); break;
			case ASupaPhaser::kDryWet	: if(dryWet) dryWet->setValue(value); break;
			case ASupaPhaser::kGain		: if(gain) gain->setValue(value); break;
			case ASupaPhaser::kInvert	: if(headButton) headButton->setValue(floorf(value * 4.0f) / 4.f); break;
			default						: break;
		}

		((ASupaPhaser *)effect)->getDisplay(index,temp);

		switch(index)
		{
			case ASupaPhaser::kAttack	: if(attackDisplay) attackDisplay->setText(temp); break;
			case ASupaPhaser::kRelease	: if(releaseDisplay) releaseDisplay->setText(temp); break;
			case ASupaPhaser::kMaxEnv	: if(envMaxDisplay) envMaxDisplay->setText(temp); break;
			case ASupaPhaser::kMinEnv	: if(envMinDisplay) envMinDisplay->setText(temp); break;
			case ASupaPhaser::kFreq		: if(freqDisplay) freqDisplay->setText(temp); break;
			case ASupaPhaser::kMinFreq	: if(lfoMinDisplay) lfoMinDisplay->setText(temp); break;
			case ASupaPhaser::kMaxFreq	: if(lfoMaxDisplay) lfoMaxDisplay->setText(temp); break;
			case ASupaPhaser::kStereo	: if(stereoDisplay) stereoDisplay->setText(temp); break;
			case ASupaPhaser::kMixture	:
				{
					long mix = (long) (100 * value );
					//long mix = (long) (100 * ((ASupaPhaser *)effect)->getParameter(tag) );

                    vstint2string(mix,temp);
					effect->getParameterLabel(ASupaPhaser::kMaxFreq,&temp[strlen(temp)]);
					if(mixtureDisplay1) mixtureDisplay1->setText(temp);
		
                    vstint2string(100 - mix,temp);
					effect->getParameterLabel(ASupaPhaser::kMaxFreq,&temp[strlen(temp)]);
					if(mixtureDisplay2) mixtureDisplay2->setText(temp);

					break;
				}
			case ASupaPhaser::kFeed		: if(feedbackDisplay) feedbackDisplay->setText(temp); break;
			case ASupaPhaser::knStages	: if(stagesDisplay) stagesDisplay->setText(temp); break;
			case ASupaPhaser::kGain		: if(gainDisplay) gainDisplay->setText(temp); break;
		}
	}
}

void ASupaEditor::valueChanged(CControl* control)
{
	long tag = control->getTag();

	if(tag >= 0 && tag < ASupaPhaser::kNumParams)
	{
		//get display...
		((ASupaPhaser *)effect)->getDisplay(tag,temp);
			
		switch(tag)
		{
			case ASupaPhaser::kAttack	: if(attackDisplay) attackDisplay->setText(temp); break;
			case ASupaPhaser::kRelease	: if(releaseDisplay) releaseDisplay->setText(temp); break;
			case ASupaPhaser::kMaxEnv	: if(envMaxDisplay) envMaxDisplay->setText(temp); break;
			case ASupaPhaser::kMinEnv	: if(envMinDisplay) envMinDisplay->setText(temp); break;
			case ASupaPhaser::kFreq		: if(freqDisplay) freqDisplay->setText(temp); break;
			case ASupaPhaser::kMinFreq	: if(lfoMinDisplay) lfoMinDisplay->setText(temp); break;
			case ASupaPhaser::kMaxFreq	: if(lfoMaxDisplay) lfoMaxDisplay->setText(temp); break;
			case ASupaPhaser::kStereo	: if(stereoDisplay) stereoDisplay->setText(temp); break;
			case ASupaPhaser::kMixture	:
				{
					//long mix = (long) (100 * value );
					long mix = (long) (100 * ((ASupaPhaser *)effect)->getParameter(tag) );

                    vstint2string(mix,temp);
					effect->getParameterLabel(ASupaPhaser::kMaxFreq,&temp[strlen(temp)]);
					if(mixtureDisplay1) mixtureDisplay1->setText(temp);
		
					//(100 - mix,temp);
					effect->getParameterLabel(ASupaPhaser::kMaxFreq,&temp[strlen(temp)]);
					if(mixtureDisplay2) mixtureDisplay2->setText(temp);

					break;
				}
			case ASupaPhaser::kFeed		: if(feedbackDisplay) feedbackDisplay->setText(temp); break;
			case ASupaPhaser::knStages	: if(stagesDisplay) stagesDisplay->setText(temp); break;
			case ASupaPhaser::kGain		: if(gainDisplay) gainDisplay->setText(temp); break;
		}

		//update control...
		effect->setParameterAutomated(tag, control->getValue ());
	}
}


	

