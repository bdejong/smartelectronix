#include "SmartelectronixDisplay.hpp"
#include "SmartelectronixDisplayEditor.h"

#include "math.h"

#include <sstream>

template <class T>
inline std::string to_string(const T& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

//-----------------------------------------------------------------------------
CSmartelectronixDisplay::CSmartelectronixDisplay(
    audioMasterCallback audioMaster)
    : AudioEffectX(audioMaster, kNumPrograms, kNumParams)
{
    setNumInputs(kNumInputChannels);
    setNumOutputs(kNumOutputChannels);

    setUniqueID('osX2');

    canProcessReplacing();

    long j;
    CPoint tmp;
    for (j = 0; j < OSC_WIDTH * 2; j += 2) {
        tmp.x = j / 2;
        tmp.y = OSC_HEIGHT * 0.5 - 1;
        peaks.push_back(tmp);
        peaks.push_back(tmp);

        tmp.x = j / 2;
        tmp.y = OSC_HEIGHT * 0.5 - 1;
        copy.push_back(tmp);
        copy.push_back(tmp);
    }

    setParameter(kTriggerSpeed, 0.5f);
    setParameter(kTriggerType, 0.f);
    setParameter(kTriggerLevel, 0.5f);
    setParameter(kTriggerLimit, 0.5f);
    setParameter(kTimeWindow, 0.75f);
    setParameter(kAmpWindow, 0.5f);
    setParameter(kSyncDraw, 0.f);
    setParameter(kChannel, 0.f);
    setParameter(kFreeze, 0.f);
    setParameter(kDCKill, 0.f);

    suspend();

    editor = new CSmartelectronixDisplayEditor(this);
}

//-----------------------------------------------------------------------------------------
CSmartelectronixDisplay::~CSmartelectronixDisplay() {}

double CSmartelectronixDisplay::timeKnobToBeats(double x)
{
    return (x + 0.005) * 4.0 + 0.5;
}

void CSmartelectronixDisplay::convertVstTimeInfo(FFXTimeInfo *ffxtime)
{
    ffxtime->isValid = false;

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
        return;

    ffxtime->isValid = true;

    // set all validity bools according to the flags returned by our VstTimeInfo request
    ffxtime->tempoIsValid = (vstTimeInfo->flags & kVstTempoValid) != 0;
    ffxtime->ppqPosIsValid = (vstTimeInfo->flags & kVstPpqPosValid) != 0;
    ffxtime->barsIsValid = (vstTimeInfo->flags & kVstBarsValid) != 0;
    ffxtime->timeSigIsValid = (vstTimeInfo->flags & kVstTimeSigValid) != 0;
    ffxtime->samplesToNextBarIsValid = (ffxtime->tempoIsValid && ffxtime->ppqPosIsValid) && (ffxtime->barsIsValid && ffxtime->timeSigIsValid);
    ffxtime->cyclePosIsValid = (vstTimeInfo->flags & kVstCyclePosValid) != 0;
    ffxtime->playbackChanged = (vstTimeInfo->flags & kVstTransportChanged) != 0;
    ffxtime->playbackIsOccuring = (vstTimeInfo->flags & kVstTransportPlaying) != 0;
    ffxtime->cycleIsActive = (vstTimeInfo->flags & kVstTransportCycleActive) != 0;

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
    if (ffxtime->barsIsValid)
    {
        ffxtime->barStartPos = vstTimeInfo->barStartPos;
    } else
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

        if (ffxtime->numSamplesToNextBar < 0) // just protecting again against wacky values
            ffxtime->numSamplesToNextBar = 0;
    }

    if (ffxtime->cyclePosIsValid)
    {
        ffxtime->cycleStartPos = vstTimeInfo->cycleStartPos;
        ffxtime->cycleEndPos = vstTimeInfo->cycleEndPos;
    }
}

//-----------------------------------------------------------------------------------------
void CSmartelectronixDisplay::processSub(float** inputs, long sampleFrames)
{
    if (SAVE[kFreeze] > 0.5) {
        suspend();
        return;
    }

    float* samples = SAVE[kChannel] > 0.5 ? inputs[1] : inputs[0];

    // some simple parameter mappings...
    float gain = powf(10.f, SAVE[kAmpWindow] * 6.f - 3.f);
    float triggerLevel = (SAVE[kTriggerLevel] * 2.f - 1.f);
    long triggerType = (long)(SAVE[kTriggerType] * kNumTriggerTypes + 0.0001);
    long triggerLimit = (long)(pow(10.f, SAVE[kTriggerLimit] * 4.f)); // [0=>1 1=>10000
    double triggerSpeed = pow(10.0, 2.5 * SAVE[kTriggerSpeed] - 5.0);
    double counterSpeed = pow(10.f, -SAVE[kTimeWindow] * 5.f + 1.5); // [0=>10 1=>0.001
    double R = 1.0 - 250.0 / getSampleRate();
    bool dcOn = SAVE[kDCKill] > 0.5f;

    FFXTimeInfo info;
    convertVstTimeInfo(&info);

    double beatSamples = info.sampleRate / info.tempoBPS;
    double loopBeats = timeKnobToBeats(SAVE[kTimeWindow]);
    if (loopBeats < 1.0) loopBeats = 1.0;
    double loopLength = floor(loopBeats) * beatSamples;
    if (triggerType == kTriggerTempo) {
        counterSpeed = OSC_WIDTH / (loopBeats * beatSamples);
    }

    for (long i = 0; i < sampleFrames; i++) {
        double t = info.samplePos + i;
        // DC filter...
        dcKill = samples[i] - dcFilterTemp + R * dcKill;

        dcFilterTemp = samples[i];

        if (fabs(dcKill) < 1e-10)
            dcKill = 0.f;

        // Gain
        float sample = dcOn ? (float)dcKill : samples[i];
        sample = clip(sample * gain, 1.f);

        // triggers

        bool trigger = false;

        switch (triggerType) {
        case kTriggerInternal: {
            // internal oscillator, nothing fancy
            triggerPhase += triggerSpeed;
            if (triggerPhase >= 1.0) {
                triggerPhase -= 1.0;
                trigger = true;
            }
            break;
        }
        case kTriggerTempo: {
            double diff = t - this->lastTriggerSamples;
            if (diff > loopLength || diff < 0) {
                trigger = true;
            }
            // retrigger when restarting playback
            if (i == 0 && info.playbackChanged && info.playbackIsOccuring) {
                trigger = true;
            }
            break;
        }
        case kTriggerRising: {
            // trigger on a rising edge
            if (sample >= triggerLevel && previousSample < triggerLevel)
                trigger = true;
            break;
        }
        case kTriggerFalling: {
            // trigger on a falling edge
            if (sample <= triggerLevel && previousSample > triggerLevel)
                trigger = true;
            break;
        }
        case kTriggerFree: {
            // trigger when we've run out of the screen area :-)
            if (index >= OSC_WIDTH)
                trigger = true;
            break;
        }
        }

        // if there's a retrigger, but too fast, kill it
        triggerLimitPhase++;
        if (trigger && triggerLimitPhase < triggerLimit && triggerType != kTriggerFree && triggerType != kTriggerInternal && triggerType != kTriggerTempo)
            trigger = false;

        // @ trigger
        if (trigger) {
            unsigned long j;

            // zero peaks after the last one
            for (j = index * 2; j < OSC_WIDTH * 2; j += 2)
                peaks[j].y = peaks[j + 1].y = OSC_HEIGHT * 0.5 - 1;

            // copy to a buffer for drawing!
            for (j = 0; j < OSC_WIDTH * 2; j++)
                copy[j].y = peaks[j].y;

            // reset everything
            index = 0;
            counter = 1.0;
            max = -MAX_FLOAT;
            min = MAX_FLOAT;
            triggerLimitPhase = 0;
            lastTriggerSamples = t;
        }

        // @ sample
        if (sample > max) {
            max = sample;
            lastIsMax = true;
        }

        if (sample < min) {
            min = sample;
            lastIsMax = false;
        }

        counter += counterSpeed;

        // @ counter
        // the counter keeps track of how many samples/pixel we have
        if (counter >= 1.0) {
            if (index < OSC_WIDTH) {
                // scale here, better than in the graphics thread :-)
                double max_Y = (OSC_HEIGHT * 0.5 - max * 0.5 * OSC_HEIGHT) - 1.0;
                double min_Y = (OSC_HEIGHT * 0.5 - min * 0.5 * OSC_HEIGHT) - 1.0;

                // thanks to David @ Plogue for this interesting hint!
                peaks[(index << 1)].y = lastIsMax ? min_Y : max_Y;
                peaks[(index << 1) + 1].y = lastIsMax ? max_Y : min_Y;

                index++;
            }

            max = -MAX_FLOAT;
            min = MAX_FLOAT;

            counter -= 1.0;
        }

        // store for edge-triggers !
        previousSample = sample;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------
bool CSmartelectronixDisplay::getEffectName(char* name)
{
    strcpy(name, "s(M)exoscope");
    return true;
}

//-----------------------------------------------------------------------------------------
bool CSmartelectronixDisplay::getVendorString(char* text)
{
    strcpy(text, "Bram @ Smartelectronix");
    return true;
}

//-----------------------------------------------------------------------------------------
bool CSmartelectronixDisplay::getProductString(char* text)
{
    strcpy(text, "s(M)exoscope");
    return true;
}

//-----------------------------------------------------------------------------------------
VstInt32 CSmartelectronixDisplay::canDo(char* text)
{
    // set the capabilities of your plugin
    if (!strcmp(text, "receiveVstEvents"))
        return 0;
    if (!strcmp(text, "receiveVstMidiEvent"))
        return 0;
    if (!strcmp(text, "receiveVstTimeInfo"))
        return 0;
    if (!strcmp(text, "plugAsChannelInsert"))
        return 1;
    if (!strcmp(text, "plugAsSend"))
        return 1;
    if (!strcmp(text, "mixDryWet"))
        return 1;
    if (!strcmp(text, "1in2out"))
        return 1;
    if (!strcmp(text, "2in2out"))
        return 1;

    return -1; // explicitly can't do; 0 => don't know
}

//-----------------------------------------------------------------------------------------
void CSmartelectronixDisplay::suspend()
{
    index = 0;
    counter = 1.0;
    lastTriggerSamples = 0.0;
    max = -MAX_FLOAT;
    min = MAX_FLOAT;
    previousSample = 0.f;
    triggerPhase = 0.f;
    triggerLimitPhase = 0;
    dcKill = dcFilterTemp = 0.0;
}

//-----------------------------------------------------------------------------------------
void CSmartelectronixDisplay::resume()
{
    index = 0;
    counter = 1.0;
    lastTriggerSamples = 0.0;
    max = -MAX_FLOAT;
    min = MAX_FLOAT;
    previousSample = 0.f;
    triggerPhase = 0.f;
    triggerLimitPhase = 0;
    dcKill = dcFilterTemp = 0.0;
}

void CSmartelectronixDisplay::process(float** inputs, float** outputs,
    VstInt32 sampleFrames)
{
    float* in1 = inputs[0];
    float* in2 = inputs[1];
    float* out1 = outputs[0];
    float* out2 = outputs[1];

    for (long i = 0; i < sampleFrames; i++) {
        out1[i] += in1[i];
        out2[i] += in2[i];
    }

    processSub(inputs, sampleFrames);
}

void CSmartelectronixDisplay::processReplacing(float** inputs, float** outputs,
    VstInt32 sampleFrames)
{
    float* in1 = inputs[0];
    float* in2 = inputs[1];
    float* out1 = outputs[0];
    float* out2 = outputs[1];

    for (long i = 0; i < sampleFrames; i++) {
        out1[i] = in1[i];
        out2[i] = in2[i];
    }

    processSub(inputs, sampleFrames);
}

//-----------------------------------------------------------------------------------------
void CSmartelectronixDisplay::setParameter(VstInt32 index, float value)
{
    SAVE[index] = value;

    if (editor)
        ((AEffGUIEditor*)editor)->setParameter(index, value);
}

//-----------------------------------------------------------------------------------------
float CSmartelectronixDisplay::getParameter(VstInt32 index)
{
    return SAVE[index];
}

//-----------------------------------------------------------------------------------------
void CSmartelectronixDisplay::getParameterName(VstInt32 index, char* label)
{
    switch (index) {
    case kTriggerSpeed:
        strcpy(label, "Internal Trigger Speed");
        break;
    case kTriggerType:
        strcpy(label, "Trigger Type");
        break;
    case kTriggerLevel:
        strcpy(label, "Trigger Level");
        break;
    case kTriggerLimit:
        strcpy(label, "Trigger Limit");
        break;
    case kTimeWindow:
        strcpy(label, "Time");
        break;
    case kAmpWindow:
        strcpy(label, "Amp");
        break;
    case kSyncDraw:
        strcpy(label, "Sync Redraw");
        break;
    case kChannel:
        strcpy(label, "Channel");
        break;
    case kFreeze:
        strcpy(label, "Freeze");
        break;
    case kDCKill:
        strcpy(label, "DC Killer");
        break;
    default:
        strcpy(label, "");
        break;
    }
}

//-----------------------------------------------------------------------------------------
void CSmartelectronixDisplay::getParameterDisplay(VstInt32 index, char* text)
{
    long triggerType = (long)(SAVE[kTriggerType] * kNumTriggerTypes + 0.0001);

    switch (index) {
    case kTriggerType: {
        std::string s = to_string(triggerType);
        std::strcpy(text, s.c_str());
        break;
    }
    case kTriggerLevel: {
        float triggerLevel = (SAVE[kTriggerLevel] * 2.f - 1.f);
        float2string(triggerLevel, text, 25);
        break;
    }
    case kTriggerLimit: {
        long triggerLimit = (long)(pow(10.f, SAVE[kTriggerLimit] * 4.f)); // [0=>1 1=>10000
        std::string s = to_string(triggerLimit);
        std::strcpy(text, s.c_str());
        break;
    }
    case kTimeWindow: {
        if (triggerType == kTriggerTempo) {
            int value = int(timeKnobToBeats(SAVE[kTimeWindow]));
            if (value < 1) value = 1;
            int2string(int(value), text, 25);
            text[1] = '/';
            text[2] = '4';
            text[3] = '\0';
        } else {
            double counterSpeed = pow(10.f, -SAVE[kTimeWindow] * 5.f + 1.5); // [0=>10 1=>0.001
            float2string((float)counterSpeed, text, 25);
        }
        break;
    }
    case kTriggerSpeed: {
        double triggerSpeed = pow(10.0, 2.5 * SAVE[kTriggerSpeed] - 5.0) * getSampleRate();
        float2string((float)triggerSpeed, text, 25);
        break;
    }
    case kAmpWindow: {
        float gain = powf(10.f, SAVE[kAmpWindow] * 6.f - 3.f);
        float2string(gain, text, 25);
        break;
    }
    case kChannel: {
        if (SAVE[index] > 0.5f)
            strcpy(text, "right");
        else
            strcpy(text, "left");

        break;
    }
    case kSyncDraw:
    case kFreeze:
    case kDCKill: {
        if (SAVE[index] > 0.5f)
            strcpy(text, "on");
        else
            strcpy(text, "off");

        break;
    }
    default:
        strcpy(text, "");
        break;
    }
}

//-----------------------------------------------------------------------------------------
void CSmartelectronixDisplay::getParameterLabel(VstInt32 index, char* label)
{
    strcpy(label, "");
}

void trim(char* text)
{
    long j = 0, i = 0;
    while (text[i]) {
        if (text[i] == ' ')
            i++;
        else
            text[j++] = text[i++];
    }
    text[j] = 0;
}

void CSmartelectronixDisplay::getDisplay(VstInt32 index, char* text)
{
    getParameterDisplay(index, text);
    trim(text);
    text[5] = 0; // hack !hack !hack !hack !hack !hack !hack !
    getParameterLabel(index, &text[strlen(text)]);
}
