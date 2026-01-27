#ifndef __madshiftaeditor
#define __madshiftaeditor

#include "dfxgui.h"


class CVerticalFillSlider : public CVerticalSlider
{
 public:
  CVerticalFillSlider(const CRect &size, CControlListener *listener, long tag, 
                      float axis, CPoint &offset, 
                      long textY = 0, long textHeight = 15)
  : CVerticalSlider(size, listener, tag, size.top, size.bottom, 0, 0, offset, kBottom), 
    axis(axis), textY(textY), textHeight(textHeight)
  {
   onColor(239, 0, 0, 0);
   offColor(0, 0, 0, 0);
   stringConvert = 0;
  }
  virtual void draw(CDrawContext *pContext);
  virtual void setStringConvert(void (*newConvert) (float value, char* string))
   { stringConvert = newConvert; }

 private:
  float axis;
  long textY, textHeight;
  CColor onColor, offColor;
  void (*stringConvert) (float value, char *string);
};



class APluginEditor : public AEffGUIEditor, public CControlListener
{
 public:
  APluginEditor(AudioEffect *effect);
  virtual ~APluginEditor();
  virtual long getRect(ERect **rect);
  virtual long open(void *ptr);
  virtual void close();
  virtual void setParameter(long index, float value);
  virtual void valueChanged(CDrawContext* context, CControl* control);
  virtual void idle();

 private:
  // controls
  CVerticalFillSlider *tuneFader;
  CVerticalFillSlider *fineTuneFader;
  CVerticalFillSlider *dryWetMixFader;
  CVerticalFillSlider *gainFader;
  CNumberBox *delayCircle;
  CNumberBox *feedbackCircle;
  CNumberBox *cutoffCircle;
  CNumberBox *resonanceCircle;
  CNumberBox *rootKeyBox;
  COnOffButton *midiModeButton;
  COnOffButton *filterModeButton;
  CKickSpot *randomizeButton;
  CKickSpot *randomizePitchButton;
  CSplashScreen *aboutSplash;
  CWebLink *smartelectronixLink;
  CWebLink *tobybearLink;
  // graphics
  CBitmap *gBackground;
  CBitmap *gFilterModeButton;
  CBitmap *gMidiModeButton;
  CBitmap *gAboutSplash;

 #if MAC
  long currentCursor;
 #endif
};


#endif
