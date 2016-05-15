#include "SupaTrigger.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "math.h"

const float fadeCoeff = (float)exp(log(0.01)/FADETIME);

//-----------------------------------------------------------------------------
SupaTrigger::SupaTrigger(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, kNumPrograms, kNumParams)
{
	leftBuffer = new float[MAXSIZE];
	rightBuffer = new float[MAXSIZE];

	setNumInputs(2);		// stereo in
	setNumOutputs(NUMBERIO);		// stereo out
	setUniqueID('GLTC');	// TODO: Change for plugin identification
	canMono();				// makes sense to feed both inputs with the same signal
	canProcessReplacing();	// supports both accumulating and replacing output

	randomize();

	for(long i=0;i<MAXSIZE;i++)
		leftBuffer[i] = rightBuffer[i] = 0.f;

	setParameter(kGranularity,0.3f);
	setParameter(kSpeed,0.25f);
	setParameter(kProbReverse,0.15f);
	setParameter(kProbSpeed,0.05f);
	setParameter(kProbRearrange,0.95f);
	setParameter(kProbSilence,0.0f);
	setParameter(kProbRepeat,0.4f);

	positionInMeasure = 0;
	granularityMask = 0;
	granularity = 0;
	gain = 0.f;
	speed = 0.f;
	first = true;

	srand( (unsigned)time( NULL ) );

	instantReverse = false;
	instantSlow = false;
	instantRepeat = false;

	previousSliceIndex = 0xffffffff;
}

//-----------------------------------------------------------------------------------------
SupaTrigger::~SupaTrigger()
{
	delete leftBuffer;
	delete rightBuffer;
}

//-----------------------------------------------------------------------------------------
void SupaTrigger::setProgramName(char *name)
{
}

//-----------------------------------------------------------------------------------------
void SupaTrigger::getProgramName(char *name)
{
	strcpy(name, "");
}

//-----------------------------------------------------------------------------------------
void SupaTrigger::setParameter(long index, float value)
{
	SAVE[index] = value;
}

//-----------------------------------------------------------------------------------------
float SupaTrigger::getParameter(long index)
{
	return SAVE[index];
}

//-----------------------------------------------------------------------------------------
void SupaTrigger::getParameterName(long index, char *label)
{
#if 1
	switch(index)
	{
	case kGranularity : strcpy(label, "# slices"); break;
	case kSpeed : strcpy(label, "slow-speed"); break;
	case kProbReverse : strcpy(label, "rev-prob"); break;
	case kProbSpeed : strcpy(label, "slow-prob"); break;
	case kProbRearrange : strcpy(label, "rearr-prob"); break;
	case kProbSilence : strcpy(label, "silence-prob"); break;
	case kProbRepeat : strcpy(label, "rep-prob"); break;
	case kInstantReverse : strcpy(label, "inst-rev"); break;
	case kInstantSpeed : strcpy(label, "inst-slow"); break;
	case kInstantRepeat : strcpy(label, "inst-rep"); break;
	}
#else
	switch(index)
	{
	case kGranularity : strcpy(label, "granularity"); break;
	case kSpeed : strcpy(label, "slow-speed"); break;
	case kProbReverse : strcpy(label, "reverse prob"); break;
	case kProbSpeed : strcpy(label, "slow prob"); break;
	case kProbRearrange : strcpy(label, "rearrange prob"); break;
	case kProbSilence : strcpy(label, "silence prob"); break;
	case kProbRepeat : strcpy(label, "repeat prob"); break;
	case kInstantReverse : strcpy(label, "instant reverse"); break;
	case kInstantSpeed : strcpy(label, "instant slow"); break;
	case kInstantRepeat : strcpy(label, "instant repeat"); break;
	}
#endif
}

//-----------------------------------------------------------------------------------------
void SupaTrigger::getParameterDisplay(long index, char *text)
{
	switch(index)
	{
	case kGranularity : long2string(1 << (long)(SAVE[index]*(BITSLIDES + 0.5f)), text);  break;
	case kSpeed : sprintf(text,"%.4f",(1.f - SAVE[kSpeed]+0.01f)*4.f); break;
	case kProbReverse : long2string((long)(SAVE[index]*100.f), text); break;
	case kProbSpeed : long2string((long)(SAVE[index]*100.f), text); break;
	case kProbRearrange : long2string((long)(SAVE[index]*100.f), text); break;
	case kProbSilence : long2string((long)(SAVE[index]*100.f), text); break;
	case kProbRepeat : long2string((long)(SAVE[index]*100.f), text); break;
	case kInstantReverse : if(SAVE[index] > 0.5f) strcpy(text, "on"); else strcpy(text,"off"); break;
	case kInstantSpeed : if(SAVE[index] > 0.5f) strcpy(text, "on"); else strcpy(text,"off"); break;
	case kInstantRepeat : if(SAVE[index] > 0.5f) strcpy(text, "on"); else strcpy(text,"off"); break;
	}
}

//-----------------------------------------------------------------------------------------
void SupaTrigger::getParameterLabel(long index, char *label)
{
	switch(index)
	{
	case kGranularity : strcpy(label, "slices/measure"); break;
	case kSpeed : strcpy(label, "s"); break;
	case kProbReverse : strcpy(label, "%"); break;
	case kProbSpeed : strcpy(label, "%"); break;
	case kProbRearrange : strcpy(label, "%"); break;
	case kProbSilence : strcpy(label, "%"); break;
	case kProbRepeat : strcpy(label, "%"); break;
	case kInstantReverse : strcpy(label, ""); break;
	case kInstantSpeed : strcpy(label, ""); break;
	case kInstantRepeat : strcpy(label, ""); break;
	}
}

//-----------------------------------------------------------------------------------------
void SupaTrigger::process(float **inputs, float **outputs, long sampleFrames, bool replacing)
{
	float *in1  =  inputs[0];
    float *in2  =  inputs[1];

	//get time info
	convertVstTimeInfo(&timeInfo);

	//if not playing, or if no timeinfo, return

	if( !(timeInfo.isValid && timeInfo.playbackIsOccuring && timeInfo.tempoIsValid && timeInfo.timeSigIsValid && timeInfo.samplesToNextBarIsValid) )
	{
		positionInMeasure = 0xffffffff;

		if(replacing)
		{
			for(unsigned long i=0;i<(unsigned)sampleFrames;i++)
			{
				outputs[0][i] = in1[i];
				outputs[1][i] = in2[i];
			}
		}
		else
		{
			for(unsigned long i=0;i<(unsigned)sampleFrames;i++)
			{
				outputs[0][i] += in1[i];
				outputs[1][i] += in2[i];
			}
		}

		return;
	}

	//rediculous tempo :-)
	if(timeInfo.tempo < 20) return;

	double samplesInMeasureDouble = ceil(timeInfo.numSamplesInBeat * timeInfo.timeSigNumerator / ( timeInfo.timeSigDenominator / 4.0));
    unsigned long samplesInMeasure = (unsigned long) samplesInMeasureDouble;

	//if someone hit 'play' or 'stop' recalc...
	if(timeInfo.playbackChanged)
	{
		positionInMeasure = (unsigned long) floor(samplesInMeasureDouble - timeInfo.numSamplesToNextBar);

		if(positionInMeasure >= samplesInMeasure)
			positionInMeasure = samplesInMeasure-1;
	}

	//some variables we need
	unsigned long granularityTmp = (unsigned long)(SAVE[kGranularity]*(BITSLIDES + 0.5f));
	unsigned long granularityMaskTmp = 0xffffffff << (BITSLIDES - granularityTmp);
	unsigned long numSamplesToNextBar = (unsigned long) ceil(timeInfo.numSamplesToNextBar);

	float speedDiff = (float)exp(log(0.01)/(((1.f - SAVE[kSpeed])+0.01f)*getSampleRate()*4.f));

	for(unsigned long i=0;i<(unsigned)sampleFrames;i++)
    {
		//reset the position if we're at a bar-border
		if(i == numSamplesToNextBar)
			positionInMeasure = 0;

		leftBuffer[positionInMeasure] = in1[i];
		rightBuffer[positionInMeasure] = in2[i];

		unsigned long sliceIndex = ((positionInMeasure * MAXSLIDES) / samplesInMeasure) & granularityMask;

		if(instantRepeat && sliceIndex != 0)
			sequencer[sliceIndex].offset = (sequencer[(sliceIndex-1) & granularityMaskTmp].offset + (sliceIndex - ((sliceIndex-1) & granularityMaskTmp))) & granularityMask;

		unsigned long displacement = sequencer[sliceIndex].offset & granularityMask;

		if(granularityMaskTmp != granularityMask)
		{
			//calculate the slice index with the new mask
			unsigned long sliceIndexTmp = (((positionInMeasure) * MAXSLIDES) / samplesInMeasure) & granularityMaskTmp;

			//on a new slice!
			if(
				(granularityTmp < granularity && sliceIndex == 0) ||
				(granularityTmp > granularity && sliceIndexTmp == 0) ||
				(sliceIndex == 0 && sliceIndexTmp == 0)
			  )
			{
				granularityMask = granularityMaskTmp;
				granularity = granularityTmp;
				sliceIndex = sliceIndexTmp;

				displacement = sequencer[sliceIndex].offset & granularityMask;
			}
		}

		if(sliceIndex != previousSliceIndex)
		{
			// on a new slice!
			instantRepeat = SAVE[kInstantRepeat] > 0.5f;
			instantReverse = SAVE[kInstantReverse] > 0.5;
			instantSlow = SAVE[kInstantSpeed] > 0.5f;

			previousSliceIndex = sliceIndex;
		}

		// gain calculation!
		{
			unsigned long sliceIndexFar = ((((positionInMeasure + FADETIME) % samplesInMeasure) * MAXSLIDES) / samplesInMeasure) & granularityMask;
			unsigned long displacementFar = sequencer[sliceIndexFar].offset & granularityMask;
			bool reverseFar = sequencer[sliceIndexFar].reverse;

			float targetGain = 1.f;

			if(sequencer[sliceIndex].silence)
				targetGain = 0.f;

			if(displacementFar != displacement || positionInMeasure + FADETIME > samplesInMeasure || reverseFar != sequencer[sliceIndex].reverse)
				targetGain = 0.f;

			gain = fadeCoeff*gain + (1.f - fadeCoeff)*targetGain;
		}

		if((sequencer[sliceIndex].reverse || instantReverse)  && displacement != 0) // we can't reverse a slice that is NOW
		{
			if(!(sequencer[sliceIndex].stop || instantSlow))
			{
				unsigned long sliceSize = samplesInMeasure >> granularity;
				unsigned long sliceStart = (positionInMeasure / sliceSize) * sliceSize;
				unsigned long sliceDiff = positionInMeasure - sliceStart;

				unsigned long sliceEnd = (sliceStart + sliceSize) - sliceDiff;
				unsigned long difference = (displacement * samplesInMeasure)/MAXSLIDES;
				unsigned long bufferIndex = difference <= sliceEnd ? sliceEnd - difference : 0;

				if(replacing)
				{
					outputs[0][i] = leftBuffer[bufferIndex] * gain;
					outputs[1][i] = rightBuffer[bufferIndex] * gain;
				}
				else
				{
					outputs[0][i] += leftBuffer[bufferIndex] * gain;
					outputs[1][i] += rightBuffer[bufferIndex] * gain;
				}

				first = true;
			}
			else
			{
				if(first)
				{
					unsigned long sliceSize = samplesInMeasure >> granularity;
					unsigned long sliceStart = (positionInMeasure / sliceSize) * sliceSize;
					unsigned long sliceDiff = positionInMeasure - sliceStart;
					unsigned long sliceEnd = (sliceStart + sliceSize) - sliceDiff;
					unsigned long difference = (displacement * samplesInMeasure)/MAXSLIDES;

					//position might be negative!!
					position = (float) (difference <= sliceEnd ? sliceEnd - difference : 0);
					speed = 1.f/speedDiff;
					first = false;
				}

				speed *= speedDiff;
				position -= speed;

				if(position < 0.f) position = 0.f;

				unsigned long bufferIndex = (unsigned long)floorf(position);
				float alpha = position - bufferIndex;

				if(replacing)
				{
					outputs[0][i] = hermiteInverse(leftBuffer,bufferIndex,alpha) * gain;
					outputs[1][i] = hermiteInverse(rightBuffer,bufferIndex,alpha) * gain;
				}
				else
				{
					outputs[0][i] += hermiteInverse(leftBuffer,bufferIndex,alpha) * gain;
					outputs[1][i] += hermiteInverse(rightBuffer,bufferIndex,alpha) * gain;
				}
			}
		}
		else
		{
			if(!(sequencer[sliceIndex].stop || instantSlow) || displacement == 0)
			{
				unsigned long difference = (displacement * samplesInMeasure)/MAXSLIDES;
				unsigned long bufferIndex = positionInMeasure > difference ? positionInMeasure - difference : 0;

				if(replacing)
				{
					outputs[0][i] = leftBuffer[bufferIndex] * gain;
					outputs[1][i] = rightBuffer[bufferIndex] * gain;
				}
				else
				{
					outputs[0][i] += leftBuffer[bufferIndex] * gain;
					outputs[1][i] += rightBuffer[bufferIndex] * gain;
				}

				first = true;
			}
			else
			{
				if(first)
				{
					unsigned long sliceSize = samplesInMeasure >> granularity;
					unsigned long sliceStart = (positionInMeasure / sliceSize) * sliceSize;

					position = (float) ((positionInMeasure) - (displacement * samplesInMeasure)/MAXSLIDES);
					speed = 1.f/speedDiff;
					first = false;
				}

				speed *= speedDiff;
				position += speed;

				unsigned long bufferIndex = (unsigned long)floorf(position);
				float alpha = position - bufferIndex;

				if(replacing)
				{
					outputs[0][i] = hermiteInverse(leftBuffer,bufferIndex,alpha) * gain;
					outputs[1][i] = hermiteInverse(rightBuffer,bufferIndex,alpha) * gain;
				}
				else
				{
					outputs[0][i] += hermiteInverse(leftBuffer,bufferIndex,alpha) * gain;
					outputs[1][i] += hermiteInverse(rightBuffer,bufferIndex,alpha) * gain;
				}
			}
		}


		positionInMeasure++;

		if(positionInMeasure >= samplesInMeasure)
		{
			randomize();
			positionInMeasure = 0;
		}

		if(fabsf(gain) < 1e-10) gain = 0.f;
    }
}

void SupaTrigger::process(float **inputs, float **outputs, long sampleFrames)
{
	process(inputs,outputs,sampleFrames,false);
}

//-----------------------------------------------------------------------------------------
void SupaTrigger::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
	process(inputs,outputs,sampleFrames,true);
}

long SupaTrigger::canDo (char* text)
{
	if (!strcmp (text, "receiveVstEvents"))    return 0;
	if (!strcmp (text, "receiveVstMidiEvent")) return 0;
	if (!strcmp (text, "receiveVstTimeInfo"))  return 1;
	if (!strcmp(text, "plugAsChannelInsert")) return 1;
	if (!strcmp(text, "plugAsSend")) return 1;
	if (!strcmp(text, "mixDryWet")) return 1;
	if (!strcmp(text, "1in2out")) return 1;
	if (!strcmp(text, "2in2out")) return 1;

	return -1;	// explicitly can't do; 0 => don't know
}

/*
	Plogue - Viande: ppqPos = 48.356
	barStarPos = 0xdeadbeef   (and flag is not valid)

	"man im fucked!"

	ok, lets look at the current time sig (current as in i hope for fuck sake it didnt change
	since ppqPOs started accumulating bars!!)

	current timesig is 7/8.. OMG!
	Ok, lets transform this to  3.5/4 for ease of this example and weve proven that
	its "technically the same, as far as actual lentgh in seconds is concerned)

	48.356 / 3.5 =  13.816 bars  (13 full bars and .816 rem)

	so you are currently at 81.6% of the way through a 7/8 bar

	to get that in samples, use the current bpm
	Plogue - Viande: use the "%" thing
	Plogue - Viande: but thats just and example
	Plogue - Viande: as you can immagine this result will be different if 48.356 is in fact not an 'accumulation of 13 x 7/8 bars"
	Plogue - Viande: + remainder
	Plogue - Viande: like if you got a 4/4 bar, a 3/4, 7/4 and 7/8 7/8
	Plogue - Viande: then your barStartposHACKED will be offsetted
	Plogue - Viande: maybe synthesize that and post it :)
*/

void SupaTrigger::convertVstTimeInfo(FFXTimeInfo *ffxtime)
{
	if (ffxtime == NULL)	// bogus
		return;

	if (this == NULL)	// totally bogus
	{
		ffxtime->isValid = false;
		return;
	}

	// get some VstTimeInfo with flags requesting all of the info that we want
	VstTimeInfo *vstTimeInfo = this->getTimeInfo(kVstTempoValid
													| kVstTransportChanged
													| kVstBarsValid
													| kVstPpqPosValid
													| kVstTimeSigValid
													| kVstCyclePosValid
													| kVstTransportPlaying
													| kVstTransportCycleActive);

	if (vstTimeInfo == NULL)
	{
		ffxtime->isValid = false;
		return;
	}

	ffxtime->isValid = true;

	// set all validity bools according to the flags returned by our VstTimeInfo request
	ffxtime->tempoIsValid				= (vstTimeInfo->flags & kVstTempoValid) != 0;
	ffxtime->ppqPosIsValid				= (vstTimeInfo->flags & kVstPpqPosValid) != 0;
	ffxtime->barsIsValid				= (vstTimeInfo->flags & kVstBarsValid) != 0;
	ffxtime->timeSigIsValid				= (vstTimeInfo->flags & kVstTimeSigValid) != 0;
	ffxtime->samplesToNextBarIsValid		= (ffxtime->tempoIsValid && ffxtime->ppqPosIsValid) && (ffxtime->barsIsValid && ffxtime->timeSigIsValid);
	ffxtime->cyclePosIsValid			= (vstTimeInfo->flags & kVstCyclePosValid) != 0;
	ffxtime->playbackChanged			= (vstTimeInfo->flags & kVstTransportChanged) != 0;
	ffxtime->playbackIsOccuring			= (vstTimeInfo->flags & kVstTransportPlaying) != 0;
	ffxtime->cycleIsActive				= (vstTimeInfo->flags & kVstTransportCycleActive) != 0;

	// these can always be counted on, unless the VstTimeInfo pointer was null
	ffxtime->samplePos = vstTimeInfo->samplePos;
	ffxtime->sampleRate = vstTimeInfo->sampleRate;

	if (ffxtime->tempoIsValid)
	{
		ffxtime->tempo = vstTimeInfo->tempo;
		ffxtime->tempoBPS = vstTimeInfo->tempo / 60.0;
		ffxtime->numSamplesInBeat = vstTimeInfo->sampleRate / ffxtime->tempoBPS;
	}

	// get the song beat position of our precise current location
	if (ffxtime->ppqPosIsValid)
		ffxtime->ppqPos = vstTimeInfo->ppqPos;

	// get the song beat position of the beginning of the previous measure
	if(ffxtime->barsIsValid)
	{
		ffxtime->barStartPos = vstTimeInfo->barStartPos;
	}
	else
	{
		ffxtime->barStartPos = vstTimeInfo->ppqPos; //????????????
	}

	// get the numerator of the time signature - this is the number of beats per measure
	if (ffxtime->timeSigIsValid)
	{
		ffxtime->timeSigNumerator = vstTimeInfo->timeSigNumerator;
		ffxtime->timeSigDenominator = vstTimeInfo->timeSigDenominator;
		// it will screw up the while loop below bigtime if timeSigNumerator isn't a positive number
		if (ffxtime->timeSigNumerator <= 0)
			ffxtime->timeSigNumerator = 4;
	}

	// do some calculations for this one
	if (ffxtime->samplesToNextBarIsValid)
	{
		double distanceToNextBarPPQ;
		// calculate the distance in beats to the upcoming measure beginning point
		if (ffxtime->barStartPos == ffxtime->ppqPos)
			distanceToNextBarPPQ = 0.0;
		else
			distanceToNextBarPPQ = ffxtime->barStartPos + (double)(ffxtime->timeSigNumerator) - ffxtime->ppqPos;

		// do this stuff because some hosts (Cubase) give kind of wacky barStartPos sometimes
		while (distanceToNextBarPPQ < 0.0)
			distanceToNextBarPPQ += (double)(ffxtime->timeSigNumerator);
		while (distanceToNextBarPPQ >= (double)(ffxtime->timeSigNumerator)) //THIS WAS > and now >=
			distanceToNextBarPPQ -= (double)(ffxtime->timeSigNumerator);

		//convert the value for the distance to the next measure from beats to samples
		//ffxtime->numSamplesToNextBar = (long) (distanceToNextBarPPQ * ffxtime->numSamplesInBeat);

		ffxtime->numSamplesToNextBar = (distanceToNextBarPPQ * vstTimeInfo->sampleRate * 60.0) / vstTimeInfo->tempo;

		if (ffxtime->numSamplesToNextBar < 0)	// just protecting again against wacky values
			ffxtime->numSamplesToNextBar = 0;
	}

	if (ffxtime->cyclePosIsValid)
	{
		ffxtime->cycleStartPos = vstTimeInfo->cycleStartPos;
		ffxtime->cycleEndPos = vstTimeInfo->cycleEndPos;
	}
}

void SupaTrigger::randomize()
{
	unsigned long granularityTmp = (unsigned long)(SAVE[kGranularity]*(BITSLIDES + 0.5f));
	unsigned long granularityMaskTmp = 0xffffffff << (BITSLIDES - granularityTmp);

	for(unsigned long i=0;i<MAXSLIDES;i++)
	{
		if((rand() % 100) < SAVE[kProbRearrange]*101.f)
		{
			if((rand() % 100) < SAVE[kProbRepeat]*101.f)
			{
				if(i != 0)
					sequencer[i].offset = (sequencer[(i-1) & granularityMaskTmp].offset + (i - ((i-1) & granularityMaskTmp))) & granularityMask;
				else
					sequencer[i].offset = 0;
			}
			else
				sequencer[i].offset = (i * (rand()%MAXSLIDES))/MAXSLIDES;
		}
		else
		{
			sequencer[i].offset = 0;
		}

		if((rand() % 100) < SAVE[kProbReverse]*100.f && sequencer[i].offset > 0)
			sequencer[i].reverse = true;
		else
			sequencer[i].reverse = false;

		if((rand() % 100) < SAVE[kProbSpeed]*101.f)
			sequencer[i].stop = true;
		else
			sequencer[i].stop = false;

		if((rand() % 100) < SAVE[kProbSilence]*101.f)
			sequencer[i].silence = true;
		else
			sequencer[i].silence = false;

		//make sure that:
		// if a slice coincides with the current slice, reverse is not true
	}
}

bool SupaTrigger::getEffectName (char* name)
{
	strcpy (name, "SupaTrigga");
	return true;
}

bool SupaTrigger::getVendorString (char* text)
{
	strcpy (text, "Bram @ Smartelectronix");
	return true;
}

bool SupaTrigger::getProductString (char* text)
{
	strcpy (text, "SupaTrigga");
	return true;
}
