#include "Label.h"

CLabel::CLabel(const CRect &R, const char *text) : CParamDisplay(R)
{
	CColor grey = {118,118,118};

	setFont(kNormalFontSmall);
	setFontColor(kWhiteCColor);
	setBackColor(grey);

	strcpy(label,"");
	setLabel(text);
}

CLabel::~CLabel()
{
}

void CLabel::draw(CDrawContext *pContext)
{
	pContext->setFillColor(backColor);
	pContext->drawRect(size);

	pContext->setFrameColor(fontColor);
	pContext->setFont(fontID);
	pContext->setFontColor(fontColor);
	pContext->drawString(label,size,kCenterText,true);
}

void CLabel::setLabel(const char *text)
{
	if(text)
		strcpy(label,text);
	setDirty();
}
