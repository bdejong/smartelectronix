#if !defined(AFX_LABEL_H__083847F7_7666_413A_BC3D_9008556151C2__INCLUDED_)
#define AFX_LABEL_H__083847F7_7666_413A_BC3D_9008556151C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "vstgui.h"

class CLabel : public CParamDisplay
{
public:
	CLabel(const CRect &R, const char *text);

	void setLabel(const char *text);
	void draw(CDrawContext *pContext);

	virtual ~CLabel();

protected:
	char label[256];
};

#endif
