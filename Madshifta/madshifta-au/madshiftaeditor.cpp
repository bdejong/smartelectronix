#include "dfxgui.h"

#include "madshifta.h"


//----------------------------------------------------------------------------- 
class MadShiftaEditor : public DfxGuiEditor
{
public:
	MadShiftaEditor(DGEditorListenerInstance inInstance);
	virtual long OpenEditor();
};


//----------------------------------------------------------------------------- 
enum {
	// control positions
	kLeftFaderWidth = 25,
	kLeftFaderHeight = 100,
	kFaderDisplayY = 66,
	kTuneFaderX = 28,
	kTuneFaderY = 98,
	kFineTuneFaderX = 68,
	kFineTuneFaderY = 98,

	kRightFaderWidth = 23,
	kRightFaderHeight = 126,
	kDryWetMixFaderX = 339,
	kDryWetMixFaderY = 98,
	kGainFaderX = 381,
	kGainFaderY = 98,

	kCircleDisplayWidth = 38,
	kCircleDisplayHeight = kCircleDisplayWidth,
	kDelayCircleX = 157,
	kDelayCircleY = 100,
	kFeedbackCircleX = kDelayCircleX + 79,
	kFeedbackCircleY = kDelayCircleY,
	kCutoffCircleX = kDelayCircleX,
	kCutoffCircleY = kDelayCircleY + 99,
	kResonanceCircleX = kFeedbackCircleX,
	kResonanceCircleY = kCutoffCircleY,

	kRootKeyBoxX = 22,
	kRootKeyBoxY = 225,
	kRootKeyBoxWidth = 34,
	kRootKeyBoxHeight = 18,

	kFilterModeButtonX = 209,
	kFilterModeButtonY = 246,
	kMidiModeButtonX = 61,
	kMidiModeButtonY = 223,

	kRandomizeX = 7,
	kRandomizeWidth = 38,
	kRandomizeY = 270,
	kRandomizeHeight = 26,
	kRandomizePitchX = 388,
	kRandomizePitchWidth = 31,
	kRandomizePitchY = 265,
	kRandomizePitchHeight = 35,

	kSplashZoneX = 9,
	kSplashZoneWidth = 414,
	kSplashZoneY = 13,
	kSplashZoneHeight = 42,

	kSmartElectronixLinkX = 289,
	kSmartElectronixLinkWidth = 131,
	kSmartElectronixLinkY = 325,
	kSmartElectronixLinkHeight = 17,
	kTobybearLinkX = 14,
	kTobybearLinkWidth = 87,
	kTobybearLinkY = 325,
	kTobybearLinkHeight = 21
};


const char * kValueDisplayTextFont = "Helvetica";
const DGColor kValueDisplayTextColor = kDGColor_white;
const float kNumberCircleTextSize = 10.0f;
const float kSliderTextSize = 9.0f;

const char * kRootKeyTextFont = "Helvetica Bold";
const float kRootKeyTextSize = 11.0f;
const DGColor kRootKeyTextColor = kDGColor_black;

const DGColor kControlBackgroundColor = kDGColor_black;
const DGColor kControlFillColor(0.937254901960784f, 0.0f, 0.0f);
const DGColor kControlFillColor_alt(0.466666666666667f, 0.0f, 0.0f);



//----------------------------------------------------------------------------- 
void percentDisplayProc_fine(float inValue, char * outText, void *);
void percentDisplayProc_fine(float inValue, char * outText, void *)
{
	sprintf(outText, "%.*f%%", (inValue < 100.0f) ? 1 : 0, inValue);
}

//----------------------------------------------------------------------------- 
void percentDisplayProc_course(float inValue, char * outText, void *);
void percentDisplayProc_course(float inValue, char * outText, void *)
{
	sprintf(outText, "%.0f%%", inValue);
}

//----------------------------------------------------------------------------- 
void tuneDisplayProc(float inValue, char * outText, void *);
void tuneDisplayProc(float inValue, char * outText, void *)
{
	int value_i = (int)inValue;
	sprintf(outText, "%s%d", (value_i > 0) ? "+" : "", value_i);
}

//----------------------------------------------------------------------------- 
void fineTuneDisplayProc(float inValue, char * outText, void *);
void fineTuneDisplayProc(float inValue, char * outText, void *)
{
	const int precision = 0;
	const float cutoff = 1.0f / powf(10.0f, (float)precision);
	sprintf(outText, "%s%.*f%%", (inValue >= cutoff) ? "+" : "", precision, inValue);
}

//----------------------------------------------------------------------------- 
void rootKeyDisplayProc(float inValue, char * outText, void *);
void rootKeyDisplayProc(float inValue, char * outText, void *)
{
	strcpy(outText, DFX_GetNameForMIDINote((long)inValue));
}

//----------------------------------------------------------------------------- 
void randomizeMadShifta(long inValue, void * inEditor);
void randomizeMadShifta(long inValue, void * inEditor)
{
	if (inEditor != NULL)
	{
		for (long i=0; i < kNumParams; i++)
		{
			switch (i)
			{
				case kRoot:
				case kMMode:
				case kDryWet:
				case kOutVol:
					break;
				default:
					((DfxGuiEditor*)inEditor)->randomizeparameter(i, true);
					break;
			}
		}
	}
}

//----------------------------------------------------------------------------- 
void randomizePitch(long inValue, void * inEditor);
void randomizePitch(long inValue, void * inEditor)
{
	if (inEditor != NULL)
	{
		((DfxGuiEditor*)inEditor)->randomizeparameter(kTune, true);
		((DfxGuiEditor*)inEditor)->randomizeparameter(kFine, true);
	}
}





//-----------------------------------------------------------------------------
class MadShiftaNumberCircle : public DGTextDisplay
{
public:
	MadShiftaNumberCircle(DfxGuiEditor * inOwnerEditor, AudioUnitParameterID inParamID, DGRect * inRegion, 
							DGValue2TextProcedure inTextProc)
	:	DGTextDisplay(inOwnerEditor, inParamID, inRegion, inTextProc, 
						NULL, NULL, kDGTextAlign_center, 
						kNumberCircleTextSize, kValueDisplayTextColor, kValueDisplayTextFont)
	{
	}
	virtual void draw(DGGraphicsContext * inContext);
};

//-----------------------------------------------------------------------------
void MadShiftaNumberCircle::draw(DGGraphicsContext * inContext)
{
	// draw the outer edge of the background circle
	CGContextRef cgContext = inContext->getPlatformGraphicsContext();
	CGRect backgroundBounds = getBounds()->convertToCGRect( inContext->getPortHeight() );
	CGContextAddEllipseInRect(cgContext, backgroundBounds);
	inContext->setFillColor(kControlFillColor_alt);
	inContext->endPath();
	inContext->fillPath();

	// draw the inner filling of the background circle
	inContext->beginPath();
	inContext->setFillColor(kControlBackgroundColor);
	const CGFloat backgroundFrameWidth = 1.0f;
	CGRect fillBounds = CGRectInset(backgroundBounds, backgroundFrameWidth, backgroundFrameWidth);
	CGContextAddEllipseInRect(cgContext, fillBounds);
	inContext->endPath();
	inContext->fillPath();

	// draw the pie portion filling
	inContext->beginPath();
	inContext->setFillColor(kControlFillColor);
	CGFloat centerX = fillBounds.origin.x + (fillBounds.size.width * 0.5);
	CGFloat centerY = fillBounds.origin.y + (fillBounds.size.height * 0.5);
	SInt32 min = GetControl32BitMinimum( getCarbonControl() );
	SInt32 max = GetControl32BitMaximum( getCarbonControl() );
	SInt32 val = GetControl32BitValue( getCarbonControl() );
	if (val <= min)
	{
		inContext->setStrokeColor(kControlFillColor_alt);
		float linePosX = floorf(centerX) + 0.5f;	// CoreGraphics lines are positioned between pixels rather than on them
		float lineStartY = fillBounds.origin.y;
		float lineEndY = roundf(centerY);
		inContext->drawLine(linePosX, lineStartY, linePosX, lineEndY, 1.0f);
	}
	else if (val >= max)
	{
		CGContextAddEllipseInRect(cgContext, fillBounds);
		inContext->fillPath();
	}
	else
	{
		double paramValue_gen = getDfxGuiEditor()->getparameter_f( getParameterID() );
		const CAAUParameter auParam = getAUVP();
		paramValue_gen = AUParameterValueToLinear(paramValue_gen, &auParam);
		CGFloat radius = fillBounds.size.width * 0.5;
		CGFloat startAngle = 0.0;
		CGFloat angle = paramValue_gen * (kDFX_PI_d * 2.0);
startAngle = kDFX_PI_d * 0.5;
angle = kDFX_PI_d;
		CGContextAddArc(cgContext, centerX, centerY, radius, startAngle, angle + startAngle, 0);
		inContext->endPath();
		inContext->fillPath();
		inContext->beginPath();
		if (paramValue_gen >= 0.5)
		{
			startAngle -= (paramValue_gen - 0.5) * (kDFX_PI_d * 2.0);
			CGContextAddArc(cgContext, centerX, centerY, radius, startAngle, angle + startAngle, 0);
		}
		else
		{
			inContext->setFillColor(kControlBackgroundColor);
			radius += backgroundFrameWidth;
			startAngle -= paramValue_gen * (kDFX_PI_d * 2.0);
			CGContextAddArc(cgContext, centerX, centerY, radius, startAngle, angle + startAngle, 0);
		}
		inContext->endPath();
		inContext->fillPath();
	}

	DGTextDisplay::draw(inContext);
}






//----------------------------------------------------------------------------- 
class MadShiftaVerticalFillSlider : public DGSlider
{
public:
	MadShiftaVerticalFillSlider(DfxGuiEditor * inOwnerEditor, AudioUnitParameterID inParamID, DGRect * inRegion, 
							DGValue2TextProcedure inTextProc, float inAxis, long inTextY = 0, long inTextHeight = 15)
	:	DGSlider(inOwnerEditor, inParamID, inRegion, kDGAxis_vertical, NULL, NULL), 
		textProc(inTextProc), axis(inAxis), textY(inTextY), textHeight(inTextHeight)
	{
	}
	virtual void draw(DGGraphicsContext * inContext);

private:
	DGValue2TextProcedure textProc;
	float axis;
	long textY, textHeight;
};

//----------------------------------------------------------------------------- 
void MadShiftaVerticalFillSlider::draw(DGGraphicsContext * inContext)
{
	// draw background
	inContext->setFillColor(kControlBackgroundColor);
	inContext->fillRect( getBounds() );

	// calculate coordinates of "on" portion
	inContext->setFillColor(kControlFillColor);
	SInt32 min = GetControl32BitMinimum(carbonControl);
	SInt32 max = GetControl32BitMaximum(carbonControl);
	SInt32 val = GetControl32BitValue(carbonControl);
	float valNorm = ((max-min) == 0) ? 0.0f : (float)(val-min) / (float)(max-min);
	DGRect onRect( getBounds() );
	long onBottom;
	if (valNorm > axis)
	{
		onRect.y = (long) (getBounds()->h * valNorm);
		onBottom = (long) (getBounds()->h * axis);
	}
	else
	{
		onRect.y = (long) (getBounds()->h * axis);
		onBottom = (long) (getBounds()->h * valNorm);
	}
	onRect.y = getBounds()->h - onRect.y;
	onBottom = getBounds()->h - onBottom;
	onRect.h = onBottom - onRect.y;
	if (onRect.h < 1)
	{
		onRect.h = 1;
		inContext->setFillColor(kControlFillColor_alt);
	}
	onRect.y += getBounds()->y;

	// draw "on" portion
	inContext->fillRect(&onRect);

	// draw the text display
	if (textProc != NULL)
	{
		char text[kDGTextDisplay_stringSize];
		text[0] = 0;
		textProc(getAUVP().GetValue(), text, NULL);

		DGRect textBounds( getBounds() );
		textBounds.y += textY;
		textBounds.h = textHeight;

		inContext->setFont(kValueDisplayTextFont, kSliderTextSize);
		inContext->setColor(kValueDisplayTextColor);
		inContext->drawText(&textBounds, text, kDGTextAlign_center);
	}
}






//----------------------------------------------------------------------------- 
DFX_EDITOR_ENTRY(MadShiftaEditor)

//----------------------------------------------------------------------------- 
MadShiftaEditor::MadShiftaEditor(DGEditorListenerInstance inInstance)
:	DfxGuiEditor(inInstance)
{
}

//----------------------------------------------------------------------------- 
long MadShiftaEditor::OpenEditor()
{
	// background image
	DGImage * backgroundImage = new DGImage("background.png", 0, this);
	SetBackgroundImage(backgroundImage);
	DGImage * filterTypeButtonImage = new DGImage("filter-type-button.png", 0, this);
	DGImage * midiModeButtonImage = new DGImage("midi-mode-button.png", 0, this);
	DGImage * aboutImage = new DGImage("info.png", 0, this);


	DGRect pos;


	// sliders
	MadShiftaVerticalFillSlider * slider;

	pos.set(kTuneFaderX, kTuneFaderY, kLeftFaderWidth, kLeftFaderHeight);
	slider = new MadShiftaVerticalFillSlider(this, kTune, &pos, tuneDisplayProc, 0.5f, kFaderDisplayY);
//slider = new DGTextDisplay(this, kTune, &pos, tuneDisplayProc, NULL, NULL, kDGTextAlign_center, kSliderTextSize, kValueDisplayTextColor, kValueDisplayTextFont);

	pos.set(kFineTuneFaderX, kFineTuneFaderY, kLeftFaderWidth, kLeftFaderHeight);
	slider = new MadShiftaVerticalFillSlider(this, kFine, &pos, fineTuneDisplayProc, 0.5f, kFaderDisplayY);
//slider = new DGTextDisplay(this, kFine, &pos, fineTuneDisplayProc, NULL, NULL, kDGTextAlign_center, kSliderTextSize, kValueDisplayTextColor, kValueDisplayTextFont);

	pos.set(kDryWetMixFaderX, kDryWetMixFaderY, kRightFaderWidth, kRightFaderHeight);
	slider = new MadShiftaVerticalFillSlider(this, kDryWet, &pos, percentDisplayProc_course, 0.0f, kFaderDisplayY);
//slider = new DGTextDisplay(this, kDryWet, &pos, percentDisplayProc_course, NULL, NULL, kDGTextAlign_center, kSliderTextSize, kValueDisplayTextColor, kValueDisplayTextFont);

	pos.set(kGainFaderX, kGainFaderY, kRightFaderWidth, kRightFaderHeight);
	slider = new MadShiftaVerticalFillSlider(this, kOutVol, &pos, percentDisplayProc_course, 0.0f, kFaderDisplayY);
//slider = new DGTextDisplay(this, kOutVol, &pos, percentDisplayProc_course, NULL, NULL, kDGTextAlign_center, kSliderTextSize, kValueDisplayTextColor, kValueDisplayTextFont);


	// number circles
	MadShiftaNumberCircle * numberCircle;

	pos.set(kDelayCircleX, kDelayCircleY, kCircleDisplayWidth, kCircleDisplayHeight);
	numberCircle = new MadShiftaNumberCircle(this, kDelayLen, &pos, percentDisplayProc_fine);

	pos.set(kFeedbackCircleX, kFeedbackCircleY, kCircleDisplayWidth, kCircleDisplayHeight);
	numberCircle = new MadShiftaNumberCircle(this, kDelayFB, &pos, percentDisplayProc_fine);

	pos.set(kCutoffCircleX, kCutoffCircleY, kCircleDisplayWidth, kCircleDisplayHeight);
	numberCircle = new MadShiftaNumberCircle(this, kCutoff, &pos, percentDisplayProc_fine);

	pos.set(kResonanceCircleX, kResonanceCircleY, kCircleDisplayWidth, kCircleDisplayHeight);
	numberCircle = new MadShiftaNumberCircle(this, kResonance, &pos, percentDisplayProc_fine);


	// text displays
	DGTextDisplay * display;

	pos.set(kRootKeyBoxX, kRootKeyBoxY, kRootKeyBoxWidth, kRootKeyBoxHeight);
	display = new DGTextDisplay(this, kRoot, &pos, rootKeyDisplayProc, NULL, NULL, kDGTextAlign_center, 
								kRootKeyTextSize, kRootKeyTextColor, kRootKeyTextFont);


	// buttons
	DGButton * button;

	pos.set(kFilterModeButtonX, kFilterModeButtonY, filterTypeButtonImage->getWidth(), filterTypeButtonImage->getHeight() / kFilterType_NumTypes);
	button = new DGButton(this, kFType, &pos, filterTypeButtonImage, kFilterType_NumTypes, kDGButtonType_incbutton);

	pos.set(kMidiModeButtonX, kMidiModeButtonY, midiModeButtonImage->getWidth(), midiModeButtonImage->getHeight() / kMidiMode_NumModes);
	button = new DGButton(this, kMMode, &pos, midiModeButtonImage, kMidiMode_NumModes, kDGButtonType_incbutton);

	pos.set(kRandomizeX, kRandomizeY, kRandomizeWidth, kRandomizeHeight);
	button = new DGButton(this, &pos, NULL, 2, kDGButtonType_pushbutton);
	button->setUserProcedure(randomizeMadShifta, this);

	pos.set(kRandomizePitchX, kRandomizePitchY, kRandomizePitchWidth, kRandomizePitchHeight);
	button = new DGButton(this, &pos, NULL, 2, kDGButtonType_pushbutton);
	button->setUserProcedure(randomizePitch, this);


	// "about" splash screen
	DGSplashScreen * splashScreen;
	pos.set(kSplashZoneX, kSplashZoneY, kSplashZoneWidth, kSplashZoneHeight);
	splashScreen = new DGSplashScreen(this, &pos, aboutImage);


	// web links
	DGWebLink * webLinkButton;

	pos.set(kSmartElectronixLinkX, kSmartElectronixLinkY, kSmartElectronixLinkWidth, kSmartElectronixLinkHeight);
	webLinkButton = new DGWebLink(this, &pos, NULL, SMARTELECTRONIX_URL);

	pos.set(kTobybearLinkX, kTobybearLinkY, kTobybearLinkWidth, kTobybearLinkHeight);
	webLinkButton = new DGWebLink(this, &pos, NULL, "http://tobybear.de/");


	return noErr;
}
