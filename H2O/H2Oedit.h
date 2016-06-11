#pragma once

#include "H2Oeffect.h"

class H2Oedit : public H2Oeffect {
public:
    H2Oedit(audioMasterCallback audioMaster);
    virtual ~H2Oedit();
    virtual void setParameter(long index, float value);
};
