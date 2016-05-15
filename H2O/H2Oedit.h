// H2Oedit.h: interface for the H2Oedit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_H2OEDIT_H__736D3460_4B5F_11D4_B567_002018B8E8B7__INCLUDED_)
#define AFX_H2OEDIT_H__736D3460_4B5F_11D4_B567_002018B8E8B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H2Oeffect.h"

class H2Oedit : public H2Oeffect
{
public:
	H2Oedit(audioMasterCallback audioMaster);
	virtual ~H2Oedit();
	virtual void setParameter (long index, float value);

};

#endif // !defined(AFX_H2OEDIT_H__736D3460_4B5F_11D4_B567_002018B8E8B7__INCLUDED_)
