#include "ShaperView.h"
#include "vstgui.h"

CShaperView::CShaperView(CRect& size, CControlListener *listener, long tag, CBitmap *background, CBitmap *handle, CShaper *pShaper) : CControl (size,listener,tag)
{
	OSDC = 0;

	this->pShaper = pShaper;
	
	hBackground = background;
	hBackground->remember();

	hHandle = handle;
	hHandle->remember();
		
	long border = 2;
	minx = (float) (hHandle->getWidth() >> 1)+1;
	miny = (float) (hHandle->getHeight() >> 1)+1;
	maxx = (float) (size.width() - border) - minx - 1;
	maxy = (float) (size.height() - border) - miny - 1;
	minx += (float) border;
	miny += (float) border;
		
	selectedIndex = -1;

	guides = false;
}

CShaperView::~CShaperView()
{
	pShaper->SetPointerToView(0);
	
	hBackground->forget();
	hHandle->forget();
	
	if(OSDC != 0)
		delete OSDC;
}

void CShaperView::mouse(CDrawContext *pContext, CPoint &where)
{
	CPoint point;
	SplinePoint p;
	pContext->getMouseLocation(point);
	
	long button = pContext->getMouseButtons ();
	
	if(((button & kApple) && (button & kLButton)) || (button & kRButton)) //remove point
	{
		ConvertFromScreen(point,p);
		
		if(onRButtonDown(p))
		{
			draw(pContext);
			listener->valueChanged(pContext,this);
			return;
		}
	}

	if(button & kLButton) //perhaps add a point, or grab one...
	{
		ConvertFromScreen(point,p);
		
		if(onLButtonDown(p))
		{
			if(selectedIndex == -1)
			{
				listener->valueChanged(pContext,this);
				return;
			}
		}
	}
	
	button = pContext->getMouseButtons();
	pContext->getMouseLocation(point);
	ConvertFromScreen(point,p);

	while(button & kLButton) //still pushing that button?
	{
		if(onMouseMove(p))
			draw(pContext);
		else
			break;
		
		listener->valueChanged(pContext,this);

		doIdleStuff();

		button = pContext->getMouseButtons();
		pContext->getMouseLocation(point);
		ConvertFromScreen(point,p);
	}
	
	ConvertFromScreen(point,p);

	if(OnLButtonUp(p))
		draw(pContext);

	listener->valueChanged(pContext,this);
}

void CShaperView::draw(CDrawContext* pDC)
{
	if(OSDC == 0)
		OSDC = new COffscreenContext(pParent,size.width(),size.height(),kBlackCColor);

	InternRedraw(); //draw on the OSDC

	OSDC->transfert(pDC,size);

	setDirty(false);
}

bool CShaperView::onLButtonDown(SplinePoint point)
{
	/*if(point.x < 0 || point.x > 1 || point.y < 0 || point.y > 1)
		return false;*/
	
	float x = point.x;
	float y = point.y;
	
	selectedIndex = -1; //no point selected
	
	float dist;
	
	for(long i=0;i<pShaper->n;i++)
	{
		dist = (x - pShaper->p[i].x)*(x-pShaper->p[i].x) + (y-pShaper->p[i].y)*(y-pShaper->p[i].y);
		if(dist < DISTANCE)
		{
			selectedIndex = i;
			
			return true;
		}
	}

	if(point.x < 0 || point.x > 1 || point.y < 0 || point.y > 1)
		return false;
	
	//we've got a new point at (x,y)!
	if((selectedIndex == -1) && (pShaper->n<maxn))
	{
		InsertPoint(x,y);
		
		this->InternRedraw();

		return true;
	}

	return false;
}

bool CShaperView::OnLButtonUp(SplinePoint point)
{
	selectedIndex = -1; //no point selected
	return false;
}

bool CShaperView::onRButtonDown(SplinePoint point)
{
	float x = point.x;
	float y = point.y;
	
	selectedIndex = -1; //no point selected
	
	float dist;
	
	for(long i=1;i<pShaper->n-1;i++) //we don't want to remove the endpoints, now do we?
	{
		dist = (x-pShaper->p[i].x)*(x-pShaper->p[i].x) + (y-pShaper->p[i].y)*(y-pShaper->p[i].y);
		if(dist < DISTANCE)
		{
			selectedIndex = i;
			break;
		}
	}
	
	if((selectedIndex != -1) && (pShaper->n>2))
	{	
		//we've hit a point, remove it
		for(long i=selectedIndex;i<pShaper->n;i++)
			pShaper->p[i]=pShaper->p[i+1];

		selectedIndex = -1;
		
		pShaper->n--;
		
		InternRedraw();
		return true;
	}
	
	return false;
}

bool CShaperView::onMouseMove(SplinePoint point)
{
	float x = (float) point.x;
	float y = (float) point.y;
		
	if(selectedIndex != -1)
	{
		if(selectedIndex == 0 || selectedIndex == pShaper->n-1)
		{
			if((y>0) && (y<1))
			{
				pShaper->p[selectedIndex].y = y;
				InternRedraw();
				return true;
			}
			else
			{
				if(y < 1) //we're under miny for sure
				{
					pShaper->p[selectedIndex].y = 0;
					InternRedraw();
					return true;
				}
				else
				{
					pShaper->p[selectedIndex].y = 1;
					InternRedraw();
					return true;
				}
			}
		}
		else
		{
			if((x > pShaper->p[selectedIndex-1].x) && (x < pShaper->p[selectedIndex+1].x))
			{
				pShaper->p[selectedIndex].x = x;
				if((y>0) && (y<1))
						pShaper->p[selectedIndex].y = y;
				else
				{
					if(y<0)
						pShaper->p[selectedIndex].y = 0;
					else
						if(y>1)
							pShaper->p[selectedIndex].y = 1;
				}

				InternRedraw();
				return true;
			}
			else
			{
				//the user will NEVER notice the small increments
				//unless he's willing to add 10000 points :)
				//and it saves us a LOT of headaches!!!
				if(x >= pShaper->p[selectedIndex + 1].x)
					pShaper->p[selectedIndex].x = pShaper->p[selectedIndex + 1].x - 0.00001f;
				else
					pShaper->p[selectedIndex].x = pShaper->p[selectedIndex - 1].x + 0.00001f;
				
				if((y>0) && (y<1))
					pShaper->p[selectedIndex].y = point.y;
				else
				{
					if(y<0)
						pShaper->p[selectedIndex].y = 0;
					else
						if(y>1)
							pShaper->p[selectedIndex].y = 1;
				}
				
				InternRedraw();
				return true;
			}
		}
	}
	return false;
}

void CShaperView::InternRedraw()
{
	//draw background
	CRect R(0,0,size.width(),size.height());
	CPoint offset(size.left,size.top);
	hBackground->draw(OSDC,R,offset);
		
	//handles & guides...
	long x,y;
	
	long dx = (long) (hHandle->getWidth()*0.5f);
	long dy = (long) (hHandle->getHeight()*0.5f);
	
	CPoint P;
	if(guides)
	{
		OSDC->setFrameColor(kWhiteCColor);
		this->ConvertToScreen(pShaper->p[0],P);
		OSDC->moveTo(P);
		for(long i=0;i<pShaper->n;i++)
		{
			this->ConvertToScreen(pShaper->p[i],P);
			OSDC->lineTo(P);
			x = (long)(P.h - dx);
			y = (long)(P.v - dy);
			
			R(x,y,x+hHandle->getWidth(),y+hHandle->getHeight());
			
			hHandle->drawTransparent(OSDC,R);
		}
	}
	else
	{
		for(long i=0;i<pShaper->n;i++)
		{
			this->ConvertToScreen(pShaper->p[i],P);
			
			x = (long)(P.h - dx);
			y = (long)(P.v - dy);
			
			R(x,y,x+hHandle->getWidth(),y+hHandle->getHeight());
			
			hHandle->drawTransparent(OSDC,R);
		}
	}

	//the spline
	if(pShaper->n >= pShaper->degree)
		this->DrawBSpline();
	else
		this->DrawSpline();
		
	setDirty(true);
}

void CShaperView::DrawSpline()
{
	OSDC->setFrameColor(kBlackCColor);

	float dt = 1/((float) N_LINES);
	SplinePoint p1,p2;
	CPoint P1,P2;

	p1 = RecursiveSpline(pShaper->p,0,pShaper->n-1,0);
	ConvertToScreen(p1,P1);
	for(float t=dt;t<=1;t+=dt)
	{
		p2 = RecursiveSpline(pShaper->p,0,pShaper->n-1,t);
		ConvertToScreen(p2,P2);
		
		OSDC->moveTo(P1);
		OSDC->lineTo(P2);
		P1 = P2;
	}

	p2 = RecursiveSpline(pShaper->p,0,pShaper->n-1,1);
	ConvertToScreen(p2,P2);
	OSDC->moveTo(P1);
	OSDC->lineTo(P2);
}

void CShaperView::DrawBSpline()
{
	OSDC->setFrameColor(kBlackCColor);
	
	bspline(pShaper->n-1,pShaper->degree,pShaper->p,lines,N_LINES);
	
	CPoint P;
	
	ConvertToScreen(lines[0],P);
	OSDC->moveTo(P);
	for(long i=1;i<N_LINES;i++)
	{
		ConvertToScreen(lines[i],P);
		OSDC->lineTo(P);
		OSDC->moveTo(P);
	}
}

float CShaperView::getValue()
{
	return 0.f;
}

void CShaperView::InsertPoint(float x, float y)
{
	long i;
	for(i=1;i<pShaper->n;i++)
		if(x < pShaper->p[i].x)
			break;
	
	for(long j=pShaper->n;j>i;j--)
		pShaper->p[j] = pShaper->p[j-1];

	pShaper->p[i].x = x;
	pShaper->p[i].y = y;
	
	selectedIndex = i;

	pShaper->n++;
}

void CShaperView::ConvertToScreen(SplinePoint p, CPoint &P)
{
	P.h = (long)(p.x*(maxx - minx) + minx);
	P.v = (long)(p.y*(maxy - miny) + miny);
}

void CShaperView::ConvertFromScreen(CPoint P, SplinePoint &p)
{
	p.x = ((float)P.h - minx - size.left)/(maxx - minx);
	p.y = ((float)P.v - miny - size.top)/(maxy - miny);
}

void CShaperView::setGuides(bool g)
{
	if(guides != g)
	{
		guides = g;
		InternRedraw();
	}
}

void CShaperView::reset()
{
	pShaper->reset();
}

bool CShaperView::getSelectedXY(float &x, float &y)
{
	if(selectedIndex >= 0 && selectedIndex < pShaper->n)
	{
		x = pShaper->p[selectedIndex].x;
		y = 1.f - pShaper->p[selectedIndex].y;
		return true;
	}
	else
	{
		return false;
	}
}
