#pragma once

#include "audioeffectx.h"
#include "vstgui.h"
#include <string>

class CLabel : public CParamDisplay {
public:
    CLabel(const CRect& R, const std::string& text);

    void setLabel(const std::string& text);

    virtual void draw(CDrawContext* pContext);

    virtual ~CLabel() {}

protected:
    std::string _label;
};
