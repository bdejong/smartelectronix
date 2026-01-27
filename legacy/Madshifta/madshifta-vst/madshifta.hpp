#ifndef __madshifta
#define __madshifta

#include <math.h>

#include "audioeffectx.h"


// some very important VST variables are defined here:
 #define p4 1e-24f // the threshold for denormalisation on Pentium 4 CPUs
 #define kID 'TBMS' // unique plugin identifier, has to be different for every plugin
 #define kChannelID "MadS" // string displayed in the VSTi channel mixer
 #define kEffectName "MadShifta" // effect name
 #define kProduct "MadShifta" // product name
 #define kVendor "Tobybear" // vendor name
 #define kVersion 1030	// version number:  major/minor/bugfix/more
 const bool kIsSynth = false; //false=audio effect, true=synth
 const long kNumInputs = 2; // number of inputs
 const long kNumOutputs = 2; // number of outputs
 const bool kCanMono = true; // can be fed with mono signals?
 const bool kCanReplacing = true; //processreplacing() is called instead of process()

 // then we have the settings for the MIDI CC controllers, i.e.
 // what dial is controlled by what CC:
 const long kCC_Tune = 68;
 const long kCC_Fine = 69;
 const long kCC_Length = 70;
 const long kCC_Reso = 71;
 const long kCC_Feedback = 72;
 const long kCC_DryWet = 73;
 const long kCC_Cutoff = 74;
 const long kCC_OutVol = 75;


 const long kNumPrograms = 16; // 16 programs per fxb bank
 const long kNumChannels = 2;    // 2 channel mode, might be expanded in future

 const long kFadeSize = 100001;
 const long kBufferSize = 65537;


 const long kNumMidi = 128;
 const long kTuneMin = -24;
 const long kTuneMax = 24;


 const char * const midiNoteNames[128] = {
  "C-1","C#-1","D-1","D#-1","E-1","F-1","F#-1","G-1","G#-1","A-1","A#-1","B-1",
  "C 0","C#0","D 0","D#0","E 0","F 0","F#0","G 0","G#0","A 0","A#0","B 0",
  "C 1","C#1","D 1","D#1","E 1","F 1","F#1","G 1","G#1","A 1","A#1","B 1",
  "C 2","C#2","D 2","D#2","E 2","F 2","F#2","G 2","G#2","A 2","A#2","B 2",
  "C 3","C#3","D 3","D#3","E 3","F 3","F#3","G 3","G#3","A 3","A#3","B 3",
  "C 4","C#4","D 4","D#4","E 4","F 4","F#4","G 4","G#4","A 4","A#4","B 4",
  "C 5","C#5","D 5","D#5","E 5","F 5","F#5","G 5","G#5","A 5","A#5","B 5",
  "C 6","C#6","D 6","D#6","E 6","F 6","F#6","G 6","G#6","A 6","A#6","B 6",
  "C 7","C#7","D 7","D#7","E 7","F 7","F#7","G 7","G#7","A 7","A#7","B 7",
  "C 8","C#8","D 8","D#8","E 8","F 8","F#8","G 8","G#8","A 8","A#8","B 8",
  "C 9","C#9","D 9","D#9","E 9","F 9","F#9","G 9"
 };



 // and finally the set of constants defining the effect's parameter set:
enum
{
 kTune,
 kFine,
 kDelayLen,
 kDelayFB,
 kCutoff,
 kResonance,
 kFType,
 kOutVol,
 kDryWet,
 kRoot,
 kMMode,
 kpar12, // reserved for future use
 kpar13,
 kpar14,
 kpar15,
 kpar16,

 kNumParams   // 0-15, can be adjusted to your wishes
};


// these are some macros for converting some of the parameters' 
// values from VST-style 0.0 to 1.0 value to their actual values
#define tuneScaled(value)   ( (int) round( ((value)*(float)(kTuneMax-kTuneMin)) + (float)kTuneMin ) )
#define fineTuneScaled(value)   ( ((value)*200.0f) - 100.0f )
#define rootKeyScaled(value)   ( (int)round( ((value)*(float)(kNumMidi-(kTuneMax-kTuneMin))) ) - kTuneMin )


class APluginProgram
{
 friend class APlugin;
 private:
  // the parameter set for one program:
  float fDryWet, fTune, fDelayLen, fDelayFB, fFine, fRoot, fMMode, 
  fCutoff, fResonance, fFType, fOutVol;
  // program name:
  char name[51];

 public:
  APluginProgram();
};



class APlugin : public AudioEffectX
{
 private:
  // the current parameter set in memory:
  float fDryWet, fTune, fDelayLen, fDelayFB, fFine, fRoot, fMMode, 
  fCutoff, fResonance, fFType, fOutVol;

  // the buffers:
  float *fade;
  float **delay, //the delay buffer
  **buffer; // the shift buffer

  // some variables for the algorithm:
  float last[kNumChannels], old1[kNumChannels], old2[kNumChannels]; // filter variables
  int semitones; // semitone shift (can be negative)
  float oldtune, // last valid MIDI tuning
  cut, reso; // current cutoff and resonance values
  unsigned long p1, p2, // output pointers
  delay_in, delay_out, // delay buffer pointers
  inp, // input pointer for the shift-buffer
  dp, // pointer increment
  nBuffer, nSize, // determine size of fade buffer
  displace, nDelay;

  int notecount; // number of notes currently held down

 public:
  APlugin(audioMasterCallback audioMaster);
  ~APlugin();
  APluginProgram *programs;
  float vu;
  void DoProcess(float *i);
  virtual void process(float **inputs, float **outputs, long sampleframes);
  virtual void processReplacing(float **inputs, float **outputs, long sampleframes);
  virtual void setProgram(long aProgram);
  virtual long canDo(char *text);
  virtual void setProgramName(char *name);
  virtual void getProgramName(char *name);
  virtual void setParameter(long index, float value);
  virtual float getParameter(long index);
  virtual float getVu();
  float DoFilter(float i, float cutoff, float res, unsigned char ch);
  virtual long processEvents(VstEvents *ev);
  virtual void resume();
  virtual void suspend();
  virtual void getParameterLabel(long index, char *text);
  virtual void getParameterDisplay(long index, char *text);
  virtual void getParameterName(long index, char *text);
  virtual bool getOutputProperties(long index, VstPinProperties *properties);
  virtual bool getProgramNameIndexed(long category, long index, char *text);
  virtual bool getEffectName(char *name);
  virtual bool getVendorString(char *text);
  virtual bool getProductString(char *text);
  virtual long getVendorVersion();
};



// this function gives you the smallest power of 2
// that is larger than/equal to "number"
inline unsigned int n_larger(unsigned int number)
{
 unsigned int numb = 1;
 while (numb < number) numb = numb << 1;
 return numb;
}

// simple approximation
inline double powerof2(double a)
{
 return exp(a * 0.69314718056);
}


#endif
