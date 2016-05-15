// H2Oedit.cpp: implementation of the H2Oedit class.
//
//////////////////////////////////////////////////////////////////////

#include "H2Oedit.h"
#include "H2Oeditor.h"

extern bool oome;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

H2Oedit::H2Oedit(audioMasterCallback audioMaster)
		:H2Oeffect(audioMaster)
{
	setUniqueID ('H2OE');
	editor = new H2Oeditor (this);
	if (!editor)
		oome = true;
}

H2Oedit::~H2Oedit()
{

}

void H2Oedit::setParameter (long index, float value)
{
	H2Oeffect::setParameter (index, value);

	if (editor)
		((AEffGUIEditor*)editor)->setParameter (index, value);
}

