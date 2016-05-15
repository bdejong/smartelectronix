// CustomVU.h: interface for the CCustomVU class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CUSTOMVU_H__5F544B25_7208_4C23_8A52_1EB2FD87F45E__INCLUDED_)
#define AFX_CUSTOMVU_H__5F544B25_7208_4C23_8A52_1EB2FD87F45E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Defines.h"
#include "vstgui.h"
#include "SmartelectronixDisplay.hpp"

class CWaveDisplay : public CControl
{
public:
	CWaveDisplay (const CRect& size, CSmartelectronixDisplay* effect, CBitmap *back, CBitmap *heads, CBitmap *readout);
	virtual ~CWaveDisplay();	
  
	virtual void draw (CDrawContext *pContext);
	void mouse(CDrawContext *pContext, CPoint &where);
	virtual void setDirty (const bool val = true);

protected:
	COffscreenContext *OSDC;

	CPoint where;

	CSmartelectronixDisplay* effect;

	CBitmap *back;
	CBitmap *heads;
	CBitmap *readout;

	unsigned char display;
};

#endif // !defined(AFX_CUSTOMVU_H__5F544B25_7208_4C23_8A52_1EB2FD87F45E__INCLUDED_)
