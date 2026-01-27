#include "Label.h"

CLabel::CLabel(CRect &R, const char *text) : CParamDisplay(R)
{
	strcpy(label,"");
	setLabel(text);
}

CLabel::~CLabel()
{
}

void CLabel::draw(CDrawContext *pContext)
{
	pContext->setFillColor(backColor);
	pContext->fillRect(size);
	pContext->setFrameColor(fontColor);
	pContext->drawRect(size);

	pContext->setFont(fontID);
	pContext->setFontColor(fontColor);
	pContext->drawString(label,size,false,kCenterText);
}

void CLabel::setLabel(const char *text)
{
	if(text)
		strcpy(label,text);
	setDirty();
}
