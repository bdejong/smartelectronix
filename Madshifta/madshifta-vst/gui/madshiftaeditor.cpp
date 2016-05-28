#include <stdio.h>
#include <stdlib.h>

#if MAC
 #include <QuickDraw.h>	// for mouse cursor stuff
#endif

#ifndef __madshiftaeditor
#include "madshiftaeditor.hpp"
#endif

#ifndef __madshifta
#include "madshifta.hpp"
#endif


enum {
 // resource IDs
 kBackgroundID = 128,
 kFilterModeButtonID,
 kMidiModeButtonID,
 kAboutSplashID,

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

 kCircleDisplayWidth = 33,
 kCircleDisplayHeight = 33,
 kDelayCircleX = 159,
 kDelayCircleY = 103,
 kFeedbackCircleX = kDelayCircleX + 79,
 kFeedbackCircleY = kDelayCircleY,
 kCutoffCircleX = kDelayCircleX,
 kCutoffCircleY = kDelayCircleY + 99,
 kResonanceCircleX = kDelayCircleX + 79,
 kResonanceCircleY = kDelayCircleY + 99,

 kRootKeyBoxX = 20,
 kRootKeyBoxY = 227,
 kRootKeyBoxWidth = 38,
 kRootKeyBoxHeight = 17,

 kFilterModeButtonX = 209,
 kFilterModeButtonY = 246,
 kMidiModeButtonX = 61,
 kMidiModeButtonY = 223,

 kRandomizeLeft = 7,
 kRandomizeRight = 45,
 kRandomizeTop = 270,
 kRandomizeBottom = 296,
 kRandomizePitchLeft = 388,
 kRandomizePitchRight = 419,
 kRandomizePitchTop = 265,
 kRandomizePitchBottom = 300,

 kSplashZoneLeft = 9,
 kSplashZoneRight = 423,
 kSplashZoneTop = 13,
 kSplashZoneBottom = 55,

 kSmartElectronixLinkLeft = 289,
 kSmartElectronixLinkRight = 420,
 kSmartElectronixLinkTop = 325,
 kSmartElectronixLinkBottom = 342,
 kTobybearLinkLeft = 14,
 kTobybearLinkRight = 101,
 kTobybearLinkTop = 325,
 kTobybearLinkBottom = 346,

 kRandomizeTag = 3000,
 kRandomizePitchTag,
 kSmartElectronixLinkTag,
 kTobybearLinkTag,

 kHandCursor = 333
};


void percentDisplayConvert(float value, char *string);
void percentDisplayConvert(float value, char *string)
{
 sprintf(string, "%d %%", (int)(value*100.0f));
}

void tuneDisplayConvert(float value, char *string);
void tuneDisplayConvert(float value, char *string)
{
 sprintf(string, "%d", tuneScaled(value));
}

void fineTuneDisplayConvert(float value, char *string);
void fineTuneDisplayConvert(float value, char *string)
{
 sprintf(string, "%d %%", (int)fineTuneScaled(value));
}

void rootKeyDisplayConvert(float value, char *string);
void rootKeyDisplayConvert(float value, char *string)
{
 strcpy(string, midiNoteNames[rootKeyScaled(value)]);
}





void CVerticalFillSlider::draw(CDrawContext *pContext)
{
 COffscreenContext *offScreen = new COffscreenContext (pParent, widthControl, heightControl, kBlackCColor);

 // draw background
 CRect wholeRect (0, 0, widthControl, heightControl);
 offScreen->setFrameColor(offColor);
 offScreen->setFillColor(offColor);
 offScreen->drawRect(wholeRect);
 offScreen->fillRect(wholeRect);

 // calc new coords of "on" portion
 CRect onRect;
 onRect.left = 0;
 onRect.right = widthControl;
 if (value > axis)
 {
  onRect.bottom = (long) (heightControl * axis);
  onRect.top = (long) (heightControl * value);
 }
 else
 {
  onRect.bottom = (long) (heightControl * value);
  onRect.top = (long) (heightControl * axis);
 }
 onRect.top = heightControl - onRect.top;
 onRect.bottom = heightControl - onRect.bottom;

 // draw "on" portion
 offScreen->setFrameColor(onColor);
 offScreen->setFillColor(onColor);
 offScreen->drawRect(onRect);
 offScreen->fillRect(onRect);

 // draw the text display
 char string[256];
 string[0] = 0;
 if (stringConvert)
  stringConvert(value, string);
 else
  sprintf(string, "%.2f", value);
 CRect textRect(0, textY, widthControl, textY + textHeight);
 offScreen->setFont(kNormalFontVerySmall, 0, kNormalFace);
 offScreen->setFontColor(kWhiteCColor);
 offScreen->drawString(string, textRect, false, kCenterText);	// XXX should it be false?

 offScreen->copyFrom(pContext, size);
 delete offScreen;

 setDirty(false);
}






APluginEditor::APluginEditor(AudioEffect *effect)
 : AEffGUIEditor(effect)
{
 frame = 0;

 // initialize the graphics pointers
 gBackground = 0;
 gFilterModeButton = 0;
 gMidiModeButton = 0;
 gAboutSplash = 0;

 // initialize the controls pointers
 tuneFader = 0;
 fineTuneFader = 0;
 dryWetMixFader = 0;
 gainFader = 0;
 delayCircle = 0;
 feedbackCircle = 0;
 cutoffCircle = 0;
 resonanceCircle = 0;
 rootKeyBox = 0;
 midiModeButton = 0;
 filterModeButton = 0;
 randomizeButton = 0;
 randomizePitchButton = 0;
 aboutSplash = 0;
 smartelectronixLink = 0;
 tobybearLink = 0;

 // initialize the value display box pointers

 // load the background bitmap
 // we don't need to load all bitmaps, this could be done when open is called
 gBackground = new CBitmap(kBackgroundID);

 // init the size of the plugin
 rect.left   = 0;
 rect.top    = 0;
 rect.right  = (short)gBackground->getWidth();
 rect.bottom = (short)gBackground->getHeight();
}

APluginEditor::~APluginEditor()
{
 // free background bitmap
 if (gBackground)
  gBackground->forget();
 gBackground = 0;
}

long APluginEditor::getRect(ERect **erect)
{
 *erect = &rect;
 return 1;
}

long APluginEditor::open(void *ptr)
{
 CPoint point(0, 0);	// for no offset
 CPoint displayOffset;


 // !!! always call this !!!
 AEffGUIEditor::open(ptr);

 // load some bitmaps
 if (!gFilterModeButton)
  gFilterModeButton = new CBitmap(kFilterModeButtonID);
 if (!gMidiModeButton)
  gMidiModeButton = new CBitmap(kMidiModeButtonID);
 if (!gAboutSplash)
  gAboutSplash = new CBitmap(kAboutSplashID);


 //--initialize the background frame--------------------------------------
 CRect size(0, 0, gBackground->getWidth(), gBackground->getHeight());
 frame = new CFrame(size, ptr, this);
 frame->setBackground(gBackground);


#if MAC
 currentCursor = 0;
#endif


 // faders

 size (kTuneFaderX, kTuneFaderY, kTuneFaderX + kLeftFaderWidth, kTuneFaderY + kLeftFaderHeight);
 tuneFader = new CVerticalFillSlider(size, this, kTune, 0.5f, point, kFaderDisplayY);
 tuneFader->setValue(effect->getParameter(kTune));
 tuneFader->setDefaultValue(0.5f);
 tuneFader->setStringConvert(tuneDisplayConvert);
 frame->addView(tuneFader);

 size (kFineTuneFaderX, kFineTuneFaderY, kFineTuneFaderX + kLeftFaderWidth, kFineTuneFaderY + kLeftFaderHeight);
 fineTuneFader = new CVerticalFillSlider(size, this, kFine, 0.5f, point, kFaderDisplayY);
 fineTuneFader->setValue(effect->getParameter(kFine));
 fineTuneFader->setDefaultValue(0.5f);
 fineTuneFader->setStringConvert(fineTuneDisplayConvert);
 frame->addView(fineTuneFader);

 size (kDryWetMixFaderX, kDryWetMixFaderY, kDryWetMixFaderX + kRightFaderWidth, kDryWetMixFaderY + kRightFaderHeight);
 dryWetMixFader = new CVerticalFillSlider(size, this, kDryWet, 0.0f, point, kFaderDisplayY);
 dryWetMixFader->setValue(effect->getParameter(kDryWet));
 dryWetMixFader->setDefaultValue(1.0f);
 dryWetMixFader->setStringConvert(percentDisplayConvert);
 frame->addView(dryWetMixFader);

 size (kGainFaderX, kGainFaderY, kGainFaderX + kRightFaderWidth, kGainFaderY + kRightFaderHeight);
 gainFader = new CVerticalFillSlider(size, this, kOutVol, 0.0f, point, kFaderDisplayY);
 gainFader->setValue(effect->getParameter(kOutVol));
 gainFader->setDefaultValue(1.0f);
 gainFader->setStringConvert(percentDisplayConvert);
 frame->addView(gainFader);


 // number boxes

 size (kDelayCircleX, kDelayCircleY, kDelayCircleX + kCircleDisplayWidth, kDelayCircleY + kCircleDisplayHeight);
 delayCircle = new CNumberBox(size, this, kDelayLen, gBackground);
 displayOffset (size.left, size.top);
 delayCircle->setBackOffset(displayOffset);
 delayCircle->setHoriAlign(kCenterText);
 delayCircle->setFont(kNormalFontSmall);
 delayCircle->setFontColor(kWhiteCColor);
 delayCircle->setValue(effect->getParameter(kDelayLen));
 delayCircle->setDefaultValue(0.0f);
 delayCircle->setStringConvert(percentDisplayConvert);
 frame->addView(delayCircle);

 size (kFeedbackCircleX, kFeedbackCircleY, kFeedbackCircleX + kCircleDisplayWidth, kFeedbackCircleY + kCircleDisplayHeight);
 feedbackCircle = new CNumberBox(size, this, kDelayFB, gBackground);
 displayOffset (size.left, size.top);
 feedbackCircle->setBackOffset(displayOffset);
 feedbackCircle->setHoriAlign(kCenterText);
 feedbackCircle->setFont(kNormalFontSmall);
 feedbackCircle->setFontColor(kWhiteCColor);
 feedbackCircle->setValue(effect->getParameter(kDelayFB));
 feedbackCircle->setDefaultValue(0.0f);
 feedbackCircle->setStringConvert(percentDisplayConvert);
 frame->addView(feedbackCircle);

 size (kCutoffCircleX, kCutoffCircleY, kCutoffCircleX + kCircleDisplayWidth, kCutoffCircleY + kCircleDisplayHeight);
 cutoffCircle = new CNumberBox(size, this, kCutoff, gBackground);
 displayOffset (size.left, size.top);
 cutoffCircle->setBackOffset(displayOffset);
 cutoffCircle->setHoriAlign(kCenterText);
 cutoffCircle->setFont(kNormalFontSmall);
 cutoffCircle->setFontColor(kWhiteCColor);
 cutoffCircle->setValue(effect->getParameter(kCutoff));
 cutoffCircle->setDefaultValue(1.0f);
 cutoffCircle->setStringConvert(percentDisplayConvert);
 frame->addView(cutoffCircle);

 size (kResonanceCircleX, kResonanceCircleY, kResonanceCircleX + kCircleDisplayWidth, kResonanceCircleY + kCircleDisplayHeight);
 resonanceCircle = new CNumberBox(size, this, kResonance, gBackground);
 displayOffset (size.left, size.top);
 resonanceCircle->setBackOffset(displayOffset);
 resonanceCircle->setHoriAlign(kCenterText);
 resonanceCircle->setFont(kNormalFontSmall);
 resonanceCircle->setFontColor(kWhiteCColor);
 resonanceCircle->setValue(effect->getParameter(kResonance));
 resonanceCircle->setDefaultValue(0.0f);
 resonanceCircle->setStringConvert(percentDisplayConvert);
 frame->addView(resonanceCircle);

 size (kRootKeyBoxX, kRootKeyBoxY, kRootKeyBoxX + kRootKeyBoxWidth, kRootKeyBoxY + kRootKeyBoxHeight);
 rootKeyBox = new CNumberBox(size, this, kRoot, gBackground);
 displayOffset (size.left, size.top);
 rootKeyBox->setBackOffset(displayOffset);
 rootKeyBox->setHoriAlign(kCenterText);
 rootKeyBox->setFont(kNormalFont);
 rootKeyBox->setFontColor(kBlackCColor);
 rootKeyBox->setTxtFace(kBoldFace);
 rootKeyBox->setValue(effect->getParameter(kRoot));
 rootKeyBox->setStringConvert(rootKeyDisplayConvert);
 frame->addView(rootKeyBox);


 // buttons

 size (kFilterModeButtonX, kFilterModeButtonY, kFilterModeButtonX + gFilterModeButton->getWidth(), kFilterModeButtonY + (gFilterModeButton->getHeight()/2));
 filterModeButton = new COnOffButton(size, this, kFType, gFilterModeButton);
 filterModeButton->setValue(effect->getParameter(kFType));
 frame->addView(filterModeButton);

 size (kMidiModeButtonX, kMidiModeButtonY, kMidiModeButtonX + gMidiModeButton->getWidth(), kMidiModeButtonY + (gMidiModeButton->getHeight()/2));
 midiModeButton = new COnOffButton(size, this, kMMode, gMidiModeButton);
 midiModeButton->setValue(effect->getParameter(kMMode));
 frame->addView(midiModeButton);

 size (kRandomizeLeft, kRandomizeTop, kRandomizeRight, kRandomizeBottom);
 displayOffset (size.left, size.top);
 randomizeButton = new CKickSpot(size, this, kRandomizeTag, gBackground, displayOffset);
 frame->addView(randomizeButton);

 size (kRandomizePitchLeft, kRandomizePitchTop, kRandomizePitchRight, kRandomizePitchBottom);
 displayOffset (size.left, size.top);
 randomizePitchButton = new CKickSpot(size, this, kRandomizePitchTag, gBackground, displayOffset);
 frame->addView(randomizePitchButton);


 // "about" splash screen

 long splashWidth = gAboutSplash->getWidth();
 long splashHeight = gAboutSplash->getHeight();
 long splashX = (gBackground->getWidth() - splashWidth) / 2;
 long splashY = (gBackground->getHeight() - splashHeight) / 2;
 CRect toDisplay (splashX, splashY, splashX + splashWidth, splashY + splashHeight);
 size (kSplashZoneLeft, kSplashZoneTop, kSplashZoneRight, kSplashZoneBottom);
 aboutSplash = new CSplashScreen(size, this, kAboutSplashID, gAboutSplash, toDisplay, point); 
 frame->addView(aboutSplash);


 // web links

 size (kSmartElectronixLinkLeft, kSmartElectronixLinkTop, kSmartElectronixLinkRight, kSmartElectronixLinkBottom);
 smartelectronixLink = new CWebLink(size, this, kSmartElectronixLinkTag, "http://www.smartelectronix.com/");
 frame->addView(smartelectronixLink);

 size (kTobybearLinkLeft, kTobybearLinkTop, kTobybearLinkRight, kTobybearLinkBottom);
 tobybearLink = new CWebLink(size, this, kTobybearLinkTag, "http://www.tobybear.de/");
 frame->addView(tobybearLink);


 return 1;
}


void APluginEditor::close()
{
 if (frame)
  delete frame;
 frame = 0;

 // free some bitmaps
 if (gFilterModeButton)
  gFilterModeButton->forget();
 gFilterModeButton = 0;
 if (gMidiModeButton)
  gMidiModeButton->forget();
 gMidiModeButton = 0;
 if (gAboutSplash)
  gAboutSplash->forget();
 gAboutSplash = 0;
}

void APluginEditor::setParameter(long index, float value)
{
 if (!frame)
  return;

 switch (index)
 {
  case kTune:
   if (tuneFader)
    tuneFader->setValue(value);
   break;
  case kFine:
   if (fineTuneFader)
    fineTuneFader->setValue(value);
   break;
  case kDelayLen:
   if (delayCircle)
    delayCircle->setValue(value);
   break;
  case kDelayFB:
   if (feedbackCircle)
    feedbackCircle->setValue(value);
   break;
  case kCutoff:
   if (cutoffCircle)
    cutoffCircle->setValue(value);
   break;
  case kResonance:
   if (resonanceCircle)
    resonanceCircle->setValue(value);
   break;
  case kFType:
   if (filterModeButton)
    filterModeButton->setValue(value);
   break;
  case kOutVol:
   if (gainFader)
    gainFader->setValue(value);
   break;
  case kDryWet:
   if (dryWetMixFader)
    dryWetMixFader->setValue(value);
   break;
  case kRoot:
   if (rootKeyBox)
    rootKeyBox->setValue(value);
   break;
  case kMMode:
   if (midiModeButton)
    midiModeButton->setValue(value);
   break;

  default:
   return;
 }

 postUpdate();
}

void APluginEditor::valueChanged(CDrawContext* context, CControl* control)
{
 long tag = control->getTag();
 float value = control->getValue();

 if ( (tag >= 0) && (tag < kNumParams) )
  effect->setParameterAutomated(tag, value);
 else if ( (tag == kRandomizeTag) && (value > 0.5f) )
 {
  for (int i=0; i < kNumParams; i++)
  {
   switch (i)
   {
    case kFType:
     effect->setParameterAutomated(i, (float) (rand()%2));
     break;
    case kRoot:
    case kMMode:
    case kDryWet:
    case kOutVol:
     break;
    default:
     effect->setParameterAutomated(i, (float)rand()/(float)RAND_MAX);
     break;
   }
  }
 }
 else if ( (tag == kRandomizePitchTag) && (value > 0.5f) )
 {
  effect->setParameterAutomated(kTune, (float)rand()/(float)RAND_MAX);
  effect->setParameterAutomated(kFine, (float)rand()/(float)RAND_MAX);
 }

 control->update(context);
}


void APluginEditor::idle()
{
 bool somethingChanged = false;

 if (frame->isOpen())
 {
// this cursor stuff wasn't working too well, so I removed it for now...
#if 0
 #if MAC
  CControl *control = (CControl*)(frame->getCurrentView());
  if ( (control == randomizeButton) || 
       (control == randomizePitchButton) || 
       (control == smartelectronixLink) || 
       (control == tobybearLink) )
  {
   if (currentCursor == 0)
   {
    CursHandle altCursor = GetCursor(kHandCursor);
    currentCursor = kHandCursor;
    if (altCursor == NULL)
    {
     altCursor = GetCursor(crossCursor);
     currentCursor = crossCursor;
    }
    if (altCursor == NULL)
     currentCursor = 0;
    else
    {
     SetCursor(*altCursor);
     somethingChanged = true;
    }
   }
  }
  else
  {
   if (currentCursor != 0)
   {
   #if !CALL_NOT_IN_CARBON
    Cursor arrow;
    GetQDGlobalsArrow(&arrow);
    SetCursor(&arrow);
   #else
    SetCursor(&(qd.arrow));
   #endif
    currentCursor = 0;
    somethingChanged = true;
   }
  }
  #endif	// if MAC
#endif

 // indicate that changed controls need to be redrawn
 if (somethingChanged)
  postUpdate();
 }

 // this is called so that idle() actually happens
 AEffGUIEditor::idle();
}

