#ifndef NOGUI_BUILD
#include "CustomSplashScreen.h"

#if WIN32
    #include <Windows.h>
    #pragma comment(lib, "Shell32.lib")
#endif

#ifdef __APPLE__
    #include <CoreFoundation/CFBundle.h>
    #include <ApplicationServices/ApplicationServices.h>
#endif
//-----------------------------------------------------------------------------
// Stolen and contaminated by Bram from Destroy Fx
//
// handy function to open up an URL in the user's default web browser
//  * Mac OS
// returns noErr (0) if successful, otherwise a non-zero error code is returned
//  * Windows
// returns a meaningless value greater than 32 if successful, 
// otherwise an error code ranging from 0 to 32 is returned
void launch_url(const char *urlstring)
{
	if (urlstring == NULL)
		return;

#ifdef __APPLE__
	CFURLRef url = CFURLCreateWithBytes (
        NULL,                        // allocator
        (UInt8*)urlstring,     // URLBytes
        strlen(urlstring),            // length
        kCFStringEncodingASCII,      // encoding
        NULL                         // baseURL
    );
    LSOpenCFURLRef(url,0);
    CFRelease(url); 
#else
    //NOTE forced this to non-unicode since the function is using char instead of wchar_t
	ShellExecuteA(NULL, "open", urlstring, NULL, NULL, SW_SHOWNORMAL);
#endif
}

//------------------------------------------------------------------------
// CSplashScreen
//------------------------------------------------------------------------
// one click draw its pixmap, an another click redraw its parent
CCustomSplashScreen::CCustomSplashScreen (const CRect &size,
                              IControlListener *listener,
                              const long     tag,
                              CBitmap *background,
                              const CRect   &toDisplay,
                              const CPoint  &offset)
:	CControl (size, listener, tag, background), 
	toDisplay (toDisplay), offset (offset)
{
	offValue = value;
	onValue = !value;
}

//------------------------------------------------------------------------
CCustomSplashScreen::~CCustomSplashScreen()
{}

//------------------------------------------------------------------------
void CCustomSplashScreen::draw(CDrawContext *pContext)
{
	if (value && getBackground())
	{
        getBackground()->draw(pContext, toDisplay, offset);
	}
	setDirty (false);
}

//------------------------------------------------------------------------
CMouseEventResult CCustomSplashScreen::onMouseDown(CPoint& where, const CButtonState& buttons)
{
    if (!getMouseEnabled())
        return CMouseEventResult::kMouseEventNotHandled;

    if (!(buttons.isLeftButton()))
        return CMouseEventResult::kMouseEventNotHandled;

	//relative
	CRect closeRect(220,100,272,113);
	CRect bramUrl(5,48,274,66);
	
	if(value == offValue)
	{
		value = onValue;
	}
	else
	{
		//if on, turn OFF
		CPoint whereRelative = where;
		
		whereRelative.offset(-toDisplay.left,-toDisplay.top);
		
		if(closeRect.pointInside(whereRelative))
			value = offValue;
		if(bramUrl.pointInside(whereRelative))
			launch_url("http://bram.smartelectronix.com/donationware.php");
	}
	

	if(value == onValue)
	{
		//ON
		if(getParentView() && getParentView()->getFrame()->setModalView(this))
		{
			keepSize = size;
			size = toDisplay;
            setDirty(true);
			if (listener)
				listener->valueChanged(this);
		}
	}
	else
	{
		//OFF
		size = keepSize;
		if(getParentView())
		{
            getParentView()->getFrame()->setModalView(NULL);
            getParentView()->getFrame()->setDirty(true);
		}
		if(listener)
			listener->valueChanged(this);
	}
	setDirty ();

    return CMouseEventResult::kMouseEventHandled;
}

//------------------------------------------------------------------------
void CCustomSplashScreen::unSplash ()
{
	setDirty ();
	value = offValue;

	size = keepSize;
	if (getParentView())
	{
		if (getParentView()->getFrame()->getModalView() == this)
		{
            getParentView()->getFrame()->setModalView(NULL);
            getParentView()->getFrame()->setDirty(true);
		}
	}
}

#endif
