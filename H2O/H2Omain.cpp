#include "public.sdk/source/vst2.x/audioeffect.h"

#include "H2Oeffect.h"

//------------------------------------------------------------------------
/** Must be implemented externally. */
AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
    // get vst version
    if (!audioMaster(0, audioMasterVersion, 0, 0, 0, 0))
        return 0; // old version

    return new H2Oeffect(audioMaster);
}
