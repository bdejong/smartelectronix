#ifndef NOGUI_BUILD
#include "CustomSplashScreen.h"


#if MAC && !defined(__MACH__)
	#include <InternetConfig.h>
	#include <Gestalt.h>
	#include <CodeFragments.h>
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
long launch_url(const char *urlstring)
{
	if (urlstring == NULL)
		return 3;

#if MAC
#ifdef __MACH__
	CFURLRef urlcfurl = CFURLCreateWithBytes(kCFAllocatorDefault, (const UInt8*)urlstring, strlen(urlstring), kCFStringEncodingASCII, NULL);
	if (urlcfurl != NULL)
	{
		OSStatus status = LSOpenCFURLRef(urlcfurl, NULL);	// try to launch the URL
		CFRelease(urlcfurl);
		return status;
	}
	return paramErr;	// couldn't create the CFURL, so return some error code
#else
	ICInstance ICconnection;
	long urlStart = 1, urlEnd, urlLength;
	long error;
	#if CALL_NOT_IN_CARBON
	long gestaltResponse;
	error = Gestalt('ICAp', &gestaltResponse);
	if (error == noErr)
	#endif
		error = ICStart(&ICconnection, '????');
	if ( (error == noErr) && (ICconnection == (void*)kUnresolvedCFragSymbolAddress) )
		error = noErr + 3;
	#if CALL_NOT_IN_CARBON
	if (error == noErr)
		error = ICFindConfigFile(ICconnection, 0, nil);
	#endif
	if (error == noErr)
	{
		// get this info for the ICLaunchURL function
		urlEnd = urlLength = (long)strlen(urlstring) + urlStart;
		if (urlLength > 255)
			urlEnd = urlLength = 255;
		// convert the URL string into a Pascal string for the ICLaunchURL function
		char pascalURL[256];
		for (int i = 1; i < urlLength; i++)
			pascalURL[i] = urlstring[i-1];	// move each char up one spot in the string array...
		pascalURL[0] = (char)urlLength;	// ... & set the Pascal string length byte
		//
		error = ICLaunchURL(ICconnection, "\phttp", pascalURL, urlLength, &urlStart, &urlEnd);
	}
	if (error == noErr)
		error = ICStop(ICconnection);
	return error;
#endif
#endif

#if WIN32
	return (long) ShellExecute(NULL, "open", urlstring, NULL, NULL, SW_SHOWNORMAL);
#endif
}

//------------------------------------------------------------------------
// CSplashScreen
//------------------------------------------------------------------------
// one click draw its pixmap, an another click redraw its parent
CCustomSplashScreen::CCustomSplashScreen (const CRect &size,
                              CControlListener *listener, 
                              long     tag,
                              CBitmap *background,
                              CRect   &toDisplay,
                              CPoint  &offset)
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
void CCustomSplashScreen::draw (CDrawContext *pContext)
{
	if (value && pBackground)
	{
		pBackground->draw(pContext, toDisplay, offset);
	}
	setDirty (false);
}

//------------------------------------------------------------------------
void CCustomSplashScreen::mouse (CDrawContext *pContext, CPoint &where)
{
	if (!bMouseEnabled)
		return;

	long button = pContext->getMouseButtons ();
	if(!(button & kLButton))
		return;

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
		if(getParent () && getParent()->setModalView (this))
		{
			keepSize = size;
			size = toDisplay;
			draw(pContext);
			if (listener)
				listener->valueChanged (pContext, this);
		}
	}
	else
	{
		//OFF
		size = keepSize;
		if(getParent())
		{
			getParent()->setModalView(NULL);
			getParent()->draw(pContext);
		}
		if(listener)
			listener->valueChanged(pContext, this);
	}
	setDirty ();
}

//------------------------------------------------------------------------
void CCustomSplashScreen::unSplash ()
{
	setDirty ();
	value = offValue;

	size = keepSize;
	if (getParent ())
	{
		if (getParent ()->getModalView () == this)
		{
			getParent ()->setModalView (NULL);
			getParent ()->redraw ();
		}
	}
}

#endif
