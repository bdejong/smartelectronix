#include "bitmurderer.h"

AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
    // get vst version
    if (!audioMaster(0, audioMasterVersion, 0, 0, 0, 0))
    {
        return 0; // old version
}

    return new BitMurderer(audioMaster);
}
