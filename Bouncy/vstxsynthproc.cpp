#ifndef __VstXSynth__
#include "vstxsynth.h"
#endif

#include "math.h"

void VstXSynth::setSampleRate (float sampleRate)
{
	AudioEffectX::setSampleRate (sampleRate);

	delayL->setSamplerate(sampleRate);
	delayR->setSamplerate(sampleRate);

	setParam();
}

void VstXSynth::resume()
{
	delayL->resume();
	delayR->resume();

	wantEvents();
}

void VstXSynth::process(float **inputs, float **outputs, long sampleFrames)
{
	VstTimeInfo *vstTimeInfo = this->getTimeInfo(kVstTempoValid);

	float newBPM;

	if ((vstTimeInfo != NULL) && (vstTimeInfo->flags & kVstTempoValid) != 0 )
	{
		newBPM = (float) vstTimeInfo->tempo;
		if(newBPM < 10.f)	//fix crappy hosts
			newBPM = 180.f;
	}
	else
		newBPM = 180.f;

	float x = (save[kMaxDelay] * 60.f * NUM_BEATS) / (BPM * MAX_DELAY);

	if(x > 1.f) x = 1.f;

	if(newBPM != BPM || isDirty)
	{
		delayL->setParameters(x,save[kDelayShape],save[kAmpShape],save[kRandAmp]);
		delayR->setParameters(x,save[kDelayShape],save[kAmpShape],save[kRandAmp]);
		BPM = newBPM;
		isDirty = false;
	}

	delayL->process(inputs[0],outputs[0],sampleFrames,false);
	delayR->process(inputs[1],outputs[1],sampleFrames,false);

#if WIN32
	if(pf != 0)
		fprintf(pf,"Buffer-size : %d\n",sampleFrames);
#endif
}

void VstXSynth::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
	VstTimeInfo *vstTimeInfo = this->getTimeInfo(kVstTempoValid);

	float newBPM;
	if (vstTimeInfo != NULL && (vstTimeInfo->flags & kVstTempoValid) != 0 )
		newBPM = (float) vstTimeInfo->tempo;
	else
		newBPM = 60.f;

	float x = (save[kMaxDelay] * 60.f * NUM_BEATS) / (BPM * MAX_DELAY);

	if(x > 1.f) x = 1.f;

	if(newBPM != BPM || isDirty)
	{
		delayL->setParameters(x,save[kDelayShape],save[kAmpShape],save[kRandAmp]);
		delayR->setParameters(x,save[kDelayShape],save[kAmpShape],save[kRandAmp]);
		BPM = newBPM;
		isDirty = false;
	}

	delayL->process(inputs[0],outputs[0],sampleFrames,true);
	delayR->process(inputs[1],outputs[1],sampleFrames,true);

#if WIN32
	if(pf != 0)
		fprintf(pf,"Buffer-size : %d\n",sampleFrames);
#endif
}
