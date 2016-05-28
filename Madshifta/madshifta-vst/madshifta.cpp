// This is the effect part of the MadShifta source code

// All administrative stuff like parameter management is handled here,
// but of course also the implementation of the actual algorithm.

// Written by Tobias Fleischer/Tobybear (www.tobybear.de),
// original algorithm source code & additional ideas by Bram from
// SmartElectronix (www.smartelectronix.com)

// Even if it is source code, this is meant to be a learning project,
// so don't even try to change just the graphics and sell it :-)

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifndef __madshifta
#include "madshifta.hpp"
#endif

#ifndef __madshiftaeditor
#include "madshiftaeditor.hpp"
#endif


APlugin::APlugin(audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, kNumPrograms, kNumParams)	// 16 programs, 16 parameters
{
 int i;

 // allocate memory for the buffers
 fade = (float*) malloc(sizeof(float) * kFadeSize);
 delay = (float**) malloc(sizeof(float*) * kNumChannels);
 buffer = (float**) malloc(sizeof(float*) * kNumChannels);
 for (i=0; i < kNumChannels; i++)
 {
  delay[i] = (float*) malloc(sizeof(float) * kBufferSize);
  buffer[i] = (float*) malloc(sizeof(float) * kBufferSize);
 }

 // create the programs
 programs = new APluginProgram[kNumPrograms];

 notecount = 0;
 
 nSize = 10;
 nBuffer = (unsigned) (1 << nSize); // same as 2^10, but faster
 // nBuffer is the buffer size of the fade buffer

 inp = 0;
 dp = 1;
 dp = dp-(1 << 12);

 suspend(); // empty the buffers

 // initialize the crossfading buffer with a raised cosine
 for (i=0; i < nBuffer; i++)
  fade[i] = 0.5f+0.5f*cosf((((float)i/(float)(nBuffer-1))-0.5f)*2.0f*3.141592f);

 editor = new APluginEditor(this);
 srand((unsigned int)time(NULL)); // important if you use the rand() function!

 // set some important variables, according to the constants
 // you specified at the beginning of this file
 hasVu(false);
 setNumInputs(kNumInputs);
 setNumOutputs(kNumOutputs);
 canMono(kCanMono);
 canProcessReplacing(kCanReplacing);
 isSynth(kIsSynth);
 setUniqueID(kID);

 // initial program names
 for (i=0; i < kNumPrograms; i++)
  sprintf(programs[i].name, "Mad Mother %02d", i+1);
 curProgram = 0;	// eh?
 setProgram(0);
}

APlugin::~APlugin()
{
 // deallocate the buffers
 free(fade);
 for (int i=0; i < kNumChannels; i++)
 {
  free(delay[i]);
  free(buffer[i]);
 }
 free(delay);
 free(buffer);

 // destroy the created programs
 delete[] programs;
}

// this is the filter section, currently a simple LP/HP filter
// is included, but this could be extended
float APlugin::DoFilter(float i, float cutoff, float res, unsigned char ch)
{
 float fb;

 fb = res + res/(1.0f-cutoff); // calculate feedback for resonance setting
 old1[ch] = old1[ch] + cutoff*(i-old1[ch]+fb*(old1[ch]-old2[ch]));
 old2[ch] = old2[ch] + cutoff*(old1[ch]-old2[ch]);
 if (fFType < 0.5f) return old2[ch];  // return lowpass filtered signal
 else return i-old2[ch]; // return highpass filtered signal
}

// This is the processing routine that is called for every sample
// from process() or processReplacing()
// In its current implementation, i[0] contains the left channel,
// i[1] the right channel, but this could be extended to more channels.
// NOTE: arrays are generally not the fastest way to access memory
// locations, but they show the structured approach better than direct
// pointer access. But feel free to change that... :-)

// The algorithm goes like this:
// out = filter(pitchshift(input + delay(out)*feedback))*volume

void APlugin::DoProcess(float *i)
{
 float a, b;
 unsigned int ul1, ul2;
 unsigned char ch;

 // increasing the delay buffer pointers, including
 // some more power-of-2 automatic wrap arounds
 delay_in = (delay_in+1) & (nDelay-1);
 delay_out = (delay_out+1) & (nDelay-1);

 ul1 = p1 >> 12; // the '12' is to keep the accuracy...
 ul2 = p2 >> 12;
 
 for (ch=0; ch < kNumChannels; ch++) // currently this is intended for 2 channels!
 {
  // write last value at delay_in in delay buffer + denormal fix
  delay[ch][delay_in] = last[ch] + p4;

  // store input value in buffer + feedback of delay buffer at delay_out
  buffer[ch][inp] = i[ch] + fDelayFB*delay[ch][delay_out] + p4;

  // apply the fading to smooth things
  a = buffer[ch][(inp-ul1) & (nBuffer-1)] * fade[ul1 & (nBuffer-1)];
  b = buffer[ch][(inp-ul2) & (nBuffer-1)] * fade[ul2 & (nBuffer-1)];

  // apply filter & denormal fix
  last[ch] = DoFilter(a+b, cut, reso, ch) + p4;

  a = fDryWet * last[ch];

  // do hard clipping, could be improved by saturation shaping:
  if (a > 1.0f) a = 1.0f; else if (a < -1.0f) a = -1.0f;

  // mix processed/unprocessed signal
  i[ch] = i[ch]*(1.0f-fDryWet) + a + p4;
 }
 p1 = p1 - dp;
 p2 = p2 - dp;

 // increase/wrap input pointer
 inp = (inp+1) & (nBuffer-1);
}

void APlugin::process(float **inputs, float **outputs, long sampleFrames)
{
 float inp[2];
 int i;

 for (i=0; i < sampleFrames; i++)
 {
  inp[0] = inputs[0][i];
  inp[1] = inputs[1][i];
  DoProcess(inp);
  outputs[0][i] += inp[0] * fOutVol;
  outputs[1][i] += inp[1] * fOutVol;
 }
}

void APlugin::processReplacing(float **inputs, float **outputs, long sampleFrames)
{
 float inp[2];
 int i;

 for (i=0; i < sampleFrames; i++)
 {
  inp[0] = inputs[0][i];
  inp[1] = inputs[1][i];
  DoProcess(inp);
  outputs[0][i] = inp[0] * fOutVol;
  outputs[1][i] = inp[1] * fOutVol;
 }
}
       
// In this procedure, incoming MIDI events are handled
long APlugin::processEvents(VstEvents *ev)
{
 long root, note, i, status;
 float s, nvol;
 VstMidiEvent *event;
 char *midiData;

 for (i=0; i < ev->numEvents; i++)
 {
  if ( (ev->events[i])->type == kVstMidiType )
  {
   event = (VstMidiEvent*) (ev->events[i]); // get current event
   midiData = event->midiData;
   status = midiData[0] & 0xF0; // channel information is removed
   if ( (status == 0x90) && (midiData[2] > 0) ) // "note on" ?
   {
    note = (midiData[1] & 0x7F); // midi note
    root = rootKeyScaled(fRoot);
    notecount++;
    if (notecount == 1) oldtune = fTune;
    s = (float)(note-root) / (float)(kTuneMax-kTuneMin) + 0.5f;
    setParameter(kTune, s); // recalculate the tuning
   }
   else if ( ((status == 0x90) && (midiData[2] == 0)) || (status == 0x80) ) // "note off" ?
   {
    notecount--;
    if (notecount <= 0)
    {
     notecount = 0;
     if (fMMode < 0.5f) setParameter(kTune, oldtune);
    }
   }
   else if (status == 0xB0) // Midi CC ?
   {
    note = midiData[1]; // Midi CC#
    nvol = (float)midiData[2] / 127.0f; // CC data
    switch (note)
    {
     case kCC_Tune: setParameter(kTune, nvol); break;
     case kCC_Fine: setParameter(kFine, nvol); break;
     case kCC_Length: setParameter(kDelayLen, nvol); break;
     case kCC_Reso: setParameter(kResonance, nvol); break;
     case kCC_Feedback: setParameter(kDelayFB, nvol); break;
     case kCC_DryWet: setParameter(kDryWet, nvol); break;
     case kCC_Cutoff: setParameter(kCutoff, nvol); break;
     case kCC_OutVol: setParameter(kOutVol, nvol); break;
     default: break;
    }
   }
  }
 }
 return 1;
}

long APlugin::canDo(char *text)
{
 if (strcmp(text,"receiveVstEvents") == 0) return 1; // can do
 if (strcmp(text,"receiveVstMidiEvent") == 0) return 1; // can do
 if (strcmp(text,"plugAsChannelInsert") == 0) return 1; // can do
 if (strcmp(text,"plugAsSend") == 0) return 1; // can do
 if (strcmp(text,"mixDryWet") == 0) return 1; // can do
 if (kNumChannels == 2)
 {
  if (strcmp(text,"2in2out") == 0) return 1; // can do
  if (kCanMono)
   if (strcmp(text,"1in2out") == 0) return 1; // can do
 }
 else if (kNumChannels == 1)
 {
  if (strcmp(text,"1in1out") == 0) return 1; // can do
 }

 return -1;	// explicitly can't do
}

APluginProgram::APluginProgram()
{
 // Here the parameters for a program are inititialized
 fDryWet = 1.0f;
 fFine = 0.5f;
 fTune = 0.5f;
 fRoot = 36.0f/100.0f;
 fMMode = 0.0f;
 fDelayLen = 0.0f;
 fDelayFB = 0.0f;
 fCutoff = 1.0f;
 fResonance = 0.0f;
 fFType = 0.0f;
 fOutVol = 1.0f;
 strcpy(name, "Init"); // set program name
}

void APlugin::setProgram(long aProgram)
{
 if ( (aProgram < 0) || (aProgram > (kNumPrograms-1)) ) return;

 curProgram = aProgram;
 // all parameters have to be set here for a program change
 setParameter(kDryWet, programs[curProgram].fDryWet);
 setParameter(kFine, programs[curProgram].fFine);
 setParameter(kTune, programs[curProgram].fTune);
 setParameter(kRoot, programs[curProgram].fRoot);
 setParameter(kMMode, programs[curProgram].fMMode);
 setParameter(kDelayLen, programs[curProgram].fDelayLen);
 setParameter(kDelayFB, programs[curProgram].fDelayFB);
 setParameter(kCutoff, programs[curProgram].fCutoff);
 setParameter(kResonance, programs[curProgram].fResonance);
 setParameter(kFType, programs[curProgram].fFType);
 setParameter(kOutVol, programs[curProgram].fOutVol);

 // tell the host to update the editor display with the new settings
 AudioEffectX::updateDisplay();
}

void APlugin::setProgramName(char *name)
{
 strcpy(programs[curProgram].name, name);
}

void APlugin::getProgramName(char *name)
{
 strcpy(name, programs[curProgram].name);
}

bool APlugin::getProgramNameIndexed(long category, long index, char *text)
{
 if (index < kNumPrograms)
 {
  strcpy(text, programs[index].name);
  return true;
 }
 return false;
}

void APlugin::suspend()
{
 // empty those buffers!
 for (int i=0; i < kNumChannels; i++)
 {
  for (int j=0; j < kBufferSize; j++)
  {
   buffer[i][j] = 0.0f;
   delay[i][j] = 0.0f;
  }
 }
}

void APlugin::resume()
{
 wantEvents(1); // important for all plugins that receive MIDI!
}

float APlugin::getVu()
{
 float cvu = vu;
 vu = 0.0f;
 return cvu;
}

void APlugin::setParameter(long index, float value)
{
 double f;

 // value HAS to be between 0 and 1 !!!
 if (value > 1.0f) value = 1.0f;
 else if (value < 0.0f) value = 0.0f;

 if (editor)
  ((AEffGUIEditor*)editor)->setParameter(index, value);

 switch (index)
 {
  case kFine:
   fFine = value; programs[curProgram].fFine = value;
   // recalculate tune and finetune values for the algorithm:
   semitones = tuneScaled(fTune);
   f = powerof2(semitones/12.0);
   dp = (unsigned long) round(((0.5f-fFine)*(f*0.25f)+f)*(float)(1 << 12));
   dp = dp-(1 << 12);
   break;
  case kTune:
   // recalculate tune and finetune values for the algorithm:
   fTune = value; programs[curProgram].fTune = value;
   semitones = tuneScaled(fTune);
   f = powerof2(semitones/12.0);
   dp = (unsigned long) round(((0.5-fFine)*(f*0.25)+f)*(double)(1 << 12));
   dp = dp-(1 << 12);
   break;
  case kDelayLen:
   fDelayLen = value; programs[curProgram].fDelayLen = value;
   // logarithmic scale for this control:
   value = round(-500.0f*log10((1.0f-value)+0.01f))/2000.0f;
   // lower boundary:
   if (value < 0.0001f) value = 0.0001f;
   displace = (unsigned long)round(44100.0f*value) + 1;
   nDelay = n_larger(displace);
   delay_in = 0;
   // nifty wrap-around:
   delay_out = (delay_in-displace) & (nDelay-1);
   inp = 0;
   p1 = 0;
   p2 = (nBuffer >> 1) << 12; // p2 starts at center of fade
   break;
  case kCutoff:
   cut = value;
   // simple boundary check for filter cutoff (to keep it stable):
   if (value > 0.99f) cut = 0.99f;
   else if (value < 0.01f) cut = 0.01f;
   fCutoff = value; programs[curProgram].fCutoff = value;
   break;
  case kResonance:
   // simple boundary check for filter resonance (to keep it stable):
   reso = value; if (value > 0.98f) reso = 0.98f;
   fResonance = value; programs[curProgram].fResonance = value;
   break;
  // these go regularly:
  case kDryWet: fDryWet = value; programs[curProgram].fDryWet = value; break;
  case kDelayFB: fDelayFB = value; programs[curProgram].fDelayFB = value; break;
  case kFType: fFType = value; programs[curProgram].fFType = value; break;
  case kRoot: fRoot = value; programs[curProgram].fRoot = value; break;
  case kMMode: fMMode = value; programs[curProgram].fMMode = value; break;
  case kOutVol: fOutVol = value; programs[curProgram].fOutVol = value; break;
  default: break;
 }
}

float APlugin::getParameter(long index)
{
 switch (index)
 {
  case kDryWet: return fDryWet;
  case kFine: return fFine;
  case kTune: return fTune;
  case kRoot: return fRoot;
  case kMMode: return fMMode;
  case kDelayLen: return fDelayLen;
  case kDelayFB: return fDelayFB;
  case kCutoff: return fCutoff;
  case kResonance: return fResonance;
  case kFType: return fFType;
  case kOutVol: return fOutVol;
  default: return 0.0f;
 }
}

void APlugin::getParameterName(long index, char *text)
{
 switch (index)
 {
  case kFine: strcpy(text, "FineTune"); break;
  case kDryWet: strcpy(text, "DryWet"); break;
  case kTune: strcpy(text, "Tune"); break;
  case kRoot: strcpy(text, "Root"); break;
  case kMMode: strcpy(text, "MidiMode"); break;
  case kDelayLen: strcpy(text, "DelayLen"); break;
  case kDelayFB: strcpy(text, "DelayFB"); break;
  case kCutoff: strcpy(text, "Cutoff"); break;
  case kResonance: strcpy(text, "Reso"); break;
  case kFType: strcpy(text, "FltType"); break;
  case kOutVol: strcpy(text, "OutVol"); break;
  default: strcpy(text, "reserved"); break;
 }
}

void APlugin::getParameterDisplay(long index, char *text)
{
 switch (index)
 {
  case kTune:
   sprintf(text, "%d", tuneScaled(fTune));
   break;
  case kFine:
   sprintf(text, "%d", (int)fineTuneScaled(fFine));
   break;
  case kRoot:
   strcpy(text, midiNoteNames[rootKeyScaled(fRoot)]);
   break;
  case kDryWet:
  case kOutVol:
  case kDelayLen:
  case kDelayFB:
  case kCutoff:
  case kResonance:
   sprintf(text, "%d", (int)(getParameter(index)*100.0f));
   break;
  case kFType:
   if (fFType < 0.5f)
    strcpy(text, "lowpass");
   else
    strcpy(text, "highpass");
   break;
  case kMMode:
   if (fMMode < 0.5f)
    strcpy(text, "back");
   else
    strcpy(text, "hold");
   break;
  default:
   strcpy(text, "-");
   break;
 }
}

void APlugin::getParameterLabel(long index, char *text)
{
 switch (index)
 {
  case kDryWet:
  case kOutVol:
  case kDelayLen:
  case kDelayFB:
  case kCutoff:
  case kResonance:
   strcpy(text, "%");
   break;
  case kTune:
   strcpy(text, "semitones");
   break;
  case kFine:
   strcpy(text, "cents");
   break;
  default:
   strcpy(text, " ");
   break;
 }
}

bool APlugin::getEffectName(char *name)
{
 strcpy(name, kEffectName);
 return true;
}

bool APlugin::getVendorString(char *text)
{
 strcpy(text, kVendor);
 return true;
}

bool APlugin::getProductString(char *text)
{
 strcpy(text, kProduct);
 return true;
}

long APlugin::getVendorVersion()
{
 return kVersion; // return version number
}

bool APlugin::getOutputProperties(long index, VstPinProperties *properties)
{
 if ( (index >= 0) && (index < kNumOutputs) )
 {
  sprintf (properties->label, "%s %ld", kChannelID, index+1);
  sprintf (properties->shortLabel, "%s %ld", kChannelID, index+1);
  properties->flags = kVstPinIsActive;
  if (index < 2)
   properties->flags = properties->flags | kVstPinIsStereo;
  return true;
 }
 return false;
}







//-----------------------------------------------------------------------------
//                                   main()                                   |
//-----------------------------------------------------------------------------

// prototype of the export function main
#if BEOS
#define main main_plugin
extern "C" __declspec(dllexport) AEffect *main_plugin(audioMasterCallback audioMaster);

#else
AEffect *main(audioMasterCallback audioMaster);
#endif

AEffect *main(audioMasterCallback audioMaster)
{
	// get vst version
	if ( !audioMaster(0, audioMasterVersion, 0, 0, 0, 0) )
		return 0;  // old version

	AudioEffect* effect = new APlugin(audioMaster);
	if (!effect)
		return 0;
	return effect->getAeffect();
}

#if WIN32
void* hInstance;
BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
{
	hInstance = hInst;
	return 1;
}
#endif

