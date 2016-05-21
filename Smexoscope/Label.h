#pragma once

#include "audioeffectx.h"
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
