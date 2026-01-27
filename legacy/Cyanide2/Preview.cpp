#include "Preview.h"
#include "CyanideEffect.h"
#include "Defines.h"

#define X_PAD 8
#define Y_PAD 8

CPreview::CPreview(CRect& size, CControlListener *listener, long tag, CBitmap *background, CShaper *pShaper) : CControl (size,listener,tag)
{
	hBackground = background;
	hBackground->remember();
	
	OSDC = 0;
	
	this->pShaper = pShaper;
	
	sinarray = new float[size.width()-X_PAD];
	tmparray = new float[size.width()-X_PAD];
	
	fillsine();
}

CPreview::~CPreview()
{
	if(OSDC != 0)
		delete OSDC;

	delete sinarray;
	delete tmparray;

	hBackground->forget();
}

void CPreview::internredraw()
{
	if(OSDC == 0)
		return;

	long x = 0;
	float y = 0.f;

	//draw background
	CRect R(0,0,size.width(),size.width());
	CPoint offset(size.left,size.top);
	hBackground->draw(OSDC,R,offset);

	OSDC->setFrameColor(kBlackCColor);
	
	//line in the middle
	CPoint P(X_PAD/2,size.height()/2);
	OSDC->moveTo(P);
	P(size.width() - (X_PAD/2),size.height()/2);
	OSDC->lineTo(P);
		
	//sinus wave
	P(X_PAD/2,size.height()/2);
	OSDC->moveTo(P);
	long i;
	for(i=0;i<size.width()-X_PAD;i++)
	{
		P(X_PAD/2 + i,(long)((size.height()/2) - (float)(size.height()-Y_PAD)*sinarray[i]*0.5f));
		OSDC->lineTo(P);
	}
	P(size.width() - (X_PAD/2),size.height()/2);
	OSDC->lineTo(P);
	
	//shaped wave
	OSDC->setFrameColor(kWhiteCColor);

	P(X_PAD/2,size.height()/2);
	OSDC->moveTo(P);
	
	//shape it, baby
	pShaper->process(sinarray,tmparray,size.width()-X_PAD);
		
	for(i=0;i<size.width()-X_PAD;i++)
	{
		P(X_PAD/2 + i,(long)((size.height()>>1) - (float)(size.height()-Y_PAD)*tmparray[i]*0.5f));
		OSDC->lineTo(P);
	}

	P(size.width() - (X_PAD>>1),size.height()>>1);
	OSDC->lineTo(P);

	setDirty(true);
}

void CPreview::draw(CDrawContext *pDC)
{
	if(OSDC == 0)
		OSDC = new COffscreenContext(pParent,size.width(),size.height(),kBlackCColor);

	if(isDirty())
		internredraw();

	OSDC->transfert(pDC,size);

	setDirty(false);
}

void CPreview::mouse(CDrawContext *pContext, CPoint &where)
{
	//internredraw();
}


void CPreview::fillsine()
{
	for(long i=0;i<size.width()-X_PAD;i++)
		sinarray[i] = (float)sin((float)i*2.0*3.1415/(float)(size.width()-X_PAD));
}
