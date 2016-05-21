#include "SmartelectronixDisplay.hpp"	// *change*

#include "audioeffect.h"
#include "vstxsynth.h"

//------------------------------------------------------------------------
/** Must be implemented externally. */
AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
  // get vst version
	if(!audioMaster (0, audioMasterVersion, 0, 0, 0, 0))
		return 0;  // old version

  return new SmartelectronixDisplay (audioMaster);
}
