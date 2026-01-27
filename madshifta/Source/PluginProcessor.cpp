#include "PluginProcessor.h"
#include "PluginEditor.h"

MadshiftaProcessor::MadshiftaProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Cache atomic parameter pointers
    tuneParam = apvts.getRawParameterValue(TUNE_ID);
    fineParam = apvts.getRawParameterValue(FINE_ID);
    delayLenParam = apvts.getRawParameterValue(DELAY_LEN_ID);
    delayFBParam = apvts.getRawParameterValue(DELAY_FB_ID);
    cutoffParam = apvts.getRawParameterValue(CUTOFF_ID);
    resonanceParam = apvts.getRawParameterValue(RESONANCE_ID);
    fTypeParam = apvts.getRawParameterValue(FTYPE_ID);
    outVolParam = apvts.getRawParameterValue(OUT_VOL_ID);
    dryWetParam = apvts.getRawParameterValue(DRY_WET_ID);
    rootParam = apvts.getRawParameterValue(ROOT_ID);
    mModeParam = apvts.getRawParameterValue(MMODE_ID);

    // Allocate buffers — same as original constructor
    fade = (float*)malloc(sizeof(float) * kFadeSize);
    delay = (float**)malloc(sizeof(float*) * kNumChannels);
    buffer = (float**)malloc(sizeof(float*) * kNumChannels);
    for (int i = 0; i < kNumChannels; i++)
    {
        delay[i] = (float*)malloc(sizeof(float) * kBufferSize);
        buffer[i] = (float*)malloc(sizeof(float) * kBufferSize);
    }

    notecount = 0;
    nSize = 10;
    nBuffer = (unsigned)(1 << nSize);

    inp = 0;
    dp = 1;
    dp = dp - (1 << 12);

    // Clear buffers
    for (int i = 0; i < kNumChannels; i++)
    {
        for (int j = 0; j < kBufferSize; j++)
        {
            buffer[i][j] = 0.0f;
            delay[i][j] = 0.0f;
        }
    }
    for (int i = 0; i < kNumChannels; i++)
    {
        last[i] = 0.0f;
        old1[i] = 0.0f;
        old2[i] = 0.0f;
    }

    // Initialize crossfade buffer with raised cosine
    for (unsigned long i = 0; i < nBuffer; i++)
        fade[i] = 0.5f + 0.5f * cosf((((float)i / (float)(nBuffer - 1)) - 0.5f) * 2.0f * 3.141592f);

    // Initialize DSP state from defaults
    recalcTune();
    recalcDelay(0.0f);
}

MadshiftaProcessor::~MadshiftaProcessor()
{
    free(fade);
    for (int i = 0; i < kNumChannels; i++)
    {
        free(delay[i]);
        free(buffer[i]);
    }
    free(delay);
    free(buffer);
}

juce::AudioProcessorValueTreeState::ParameterLayout MadshiftaProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(TUNE_ID, 1), "Tune",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float v, int) { return juce::String(tuneScaled(v)) + " semitones"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FINE_ID, 1), "FineTune",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float v, int) { return juce::String(static_cast<int>(fineTuneScaled(v))) + " cents"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(DELAY_LEN_ID, 1), "DelayLen",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float v, int) { return juce::String(static_cast<int>(v * 100.0f)) + "%"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(DELAY_FB_ID, 1), "DelayFB",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float v, int) { return juce::String(static_cast<int>(v * 100.0f)) + "%"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(CUTOFF_ID, 1), "Cutoff",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 1.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float v, int) { return juce::String(static_cast<int>(v * 100.0f)) + "%"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(RESONANCE_ID, 1), "Reso",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float v, int) { return juce::String(static_cast<int>(v * 100.0f)) + "%"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FTYPE_ID, 1), "FltType",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float v, int) { return v < 0.5f ? juce::String("lowpass") : juce::String("highpass"); },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(OUT_VOL_ID, 1), "OutVol",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 1.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float v, int) { return juce::String(static_cast<int>(v * 100.0f)) + "%"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(DRY_WET_ID, 1), "DryWet",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 1.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float v, int) { return juce::String(static_cast<int>(v * 100.0f)) + "%"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ROOT_ID, 1), "Root",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.36f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float v, int) {
            int idx = rootKeyScaled(v);
            idx = juce::jlimit(0, 127, idx);
            return juce::String(midiNoteNames[idx]);
        },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(MMODE_ID, 1), "MidiMode",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float v, int) { return v < 0.5f ? juce::String("back") : juce::String("hold"); },
        nullptr));

    return { params.begin(), params.end() };
}

// DSP methods — preserved exactly from madshifta.cpp

float MadshiftaProcessor::DoFilter(float i, float cutoff, float res, unsigned char ch)
{
    float fb;
    fb = res + res / (1.0f - cutoff);
    old1[ch] = old1[ch] + cutoff * (i - old1[ch] + fb * (old1[ch] - old2[ch]));
    old2[ch] = old2[ch] + cutoff * (old1[ch] - old2[ch]);
    if (fFType < 0.5f) return old2[ch];
    else return i - old2[ch];
}

void MadshiftaProcessor::DoProcess(float* i)
{
    float a, b;
    unsigned int ul1, ul2;
    unsigned char ch;

    delay_in = (delay_in + 1) & (nDelay - 1);
    delay_out = (delay_out + 1) & (nDelay - 1);

    ul1 = p1 >> 12;
    ul2 = p2 >> 12;

    for (ch = 0; ch < kNumChannels; ch++)
    {
        delay[ch][delay_in] = last[ch] + p4;
        buffer[ch][inp] = i[ch] + fDelayFB * delay[ch][delay_out] + p4;

        a = buffer[ch][(inp - ul1) & (nBuffer - 1)] * fade[ul1 & (nBuffer - 1)];
        b = buffer[ch][(inp - ul2) & (nBuffer - 1)] * fade[ul2 & (nBuffer - 1)];

        last[ch] = DoFilter(a + b, cut, reso, ch) + p4;

        a = fDryWet * last[ch];

        if (a > 1.0f) a = 1.0f; else if (a < -1.0f) a = -1.0f;

        i[ch] = i[ch] * (1.0f - fDryWet) + a + p4;
    }
    p1 = p1 - dp;
    p2 = p2 - dp;

    inp = (inp + 1) & (nBuffer - 1);
}

void MadshiftaProcessor::recalcTune()
{
    semitones = tuneScaled(fTune);
    recalcTuneFromSemitones(semitones);
}

void MadshiftaProcessor::recalcTuneFromSemitones(int semis)
{
    double f = powerof2((double)semis / 12.0);
    dp = (unsigned long)round(((fFine - 0.5) * (f * 0.25) + f) * (double)(1 << 12));
    dp = dp - (1 << 12);
}

void MadshiftaProcessor::recalcDelay(float value)
{
    value = static_cast<float>(round(-500.0f * log10((1.0f - value) + 0.01f)) / 2000.0f);
    if (value < 0.0001f) value = 0.0001f;
    displace = (unsigned long)round(44100.0f * value) + 1;
    nDelay = n_larger(static_cast<unsigned int>(displace));
    delay_in = 0;
    delay_out = (delay_in - displace) & (nDelay - 1);
    inp = 0;
    p1 = 0;
    p2 = (nBuffer >> 1) << 12;
}

// AudioProcessor methods

const juce::String MadshiftaProcessor::getName() const { return JucePlugin_Name; }
bool MadshiftaProcessor::acceptsMidi() const { return true; }
bool MadshiftaProcessor::producesMidi() const { return false; }
bool MadshiftaProcessor::isMidiEffect() const { return false; }
double MadshiftaProcessor::getTailLengthSeconds() const { return 0.0; }
int MadshiftaProcessor::getNumPrograms() { return 1; }
int MadshiftaProcessor::getCurrentProgram() { return 0; }
void MadshiftaProcessor::setCurrentProgram(int) {}
const juce::String MadshiftaProcessor::getProgramName(int) { return {}; }
void MadshiftaProcessor::changeProgramName(int, const juce::String&) {}

void MadshiftaProcessor::prepareToPlay(double /*sampleRate*/, int /*samplesPerBlock*/)
{
    // Clear buffers on prepare
    for (int i = 0; i < kNumChannels; i++)
    {
        for (int j = 0; j < kBufferSize; j++)
        {
            buffer[i][j] = 0.0f;
            delay[i][j] = 0.0f;
        }
        last[i] = 0.0f;
        old1[i] = 0.0f;
        old2[i] = 0.0f;
    }
}

void MadshiftaProcessor::releaseResources() {}

bool MadshiftaProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void MadshiftaProcessor::processBlock(juce::AudioBuffer<float>& audioBuffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Load parameters every block
    float newFine = fineParam->load();
    float newDelayLen = delayLenParam->load();
    fDelayFB = delayFBParam->load();
    float newCutoff = cutoffParam->load();
    float newResonance = resonanceParam->load();
    fFType = fTypeParam->load();
    fOutVol = outVolParam->load();
    fDryWet = dryWetParam->load();
    fRoot = rootParam->load();
    fMMode = mModeParam->load();

    float newTune = tuneParam->load();

    // Recalc tune if changed (only when no MIDI notes are active)
    if (notecount == 0 && (newTune != fTune || newFine != fFine))
    {
        fTune = newTune;
        fFine = newFine;
        recalcTune();
        currentTuneDisplay.store(semitones);
    }
    else if (notecount == 0)
    {
        // Keep fTune/fFine in sync even if no recalc needed
        fTune = newTune;
        fFine = newFine;
    }
    else
    {
        // During MIDI, still update fFine so fine tune knob works
        fFine = newFine;
    }

    // Recalc delay if changed
    if (newDelayLen != fDelayLen)
    {
        fDelayLen = newDelayLen;
        recalcDelay(fDelayLen);
    }

    // Cutoff clamping
    cut = newCutoff;
    if (cut > 0.99f) cut = 0.99f;
    else if (cut < 0.01f) cut = 0.01f;
    fCutoff = newCutoff;

    // Resonance clamping
    reso = newResonance;
    if (reso > 0.98f) reso = 0.98f;
    fResonance = newResonance;

    // Process MIDI
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            int note = msg.getNoteNumber();
            notecount++;
            if (notecount == 1)
                oldtune = semitones; // save current tune in semitones
            // AU behavior: set tune to the note number directly
            // This bypasses the -24..+24 range — pitch is calculated from the note
            recalcTuneFromSemitones(note);
            lastMidiNote.store(note);
            currentTuneDisplay.store(note);
        }
        else if (msg.isNoteOff())
        {
            notecount--;
            if (notecount <= 0)
            {
                notecount = 0;
                lastMidiNote.store(-1);
                if (fMMode < 0.5f) // "Back" mode
                {
                    // Restore old tune
                    recalcTuneFromSemitones(oldtune);
                    currentTuneDisplay.store(tuneScaled(fTune));
                }
                else
                {
                    // "Hold" mode — keep current pitch
                    currentTuneDisplay.store(tuneScaled(fTune));
                }
            }
        }
        else if (msg.isController())
        {
            int cc = msg.getControllerNumber();
            float nvol = msg.getControllerValue() / 127.0f;
            juce::RangedAudioParameter* param = nullptr;
            switch (cc)
            {
                case kCC_Tune:     param = apvts.getParameter(TUNE_ID); break;
                case kCC_Fine:     param = apvts.getParameter(FINE_ID); break;
                case kCC_Length:   param = apvts.getParameter(DELAY_LEN_ID); break;
                case kCC_Reso:     param = apvts.getParameter(RESONANCE_ID); break;
                case kCC_Feedback: param = apvts.getParameter(DELAY_FB_ID); break;
                case kCC_DryWet:   param = apvts.getParameter(DRY_WET_ID); break;
                case kCC_Cutoff:   param = apvts.getParameter(CUTOFF_ID); break;
                case kCC_OutVol:   param = apvts.getParameter(OUT_VOL_ID); break;
                default: break;
            }
            if (param)
                param->setValueNotifyingHost(nvol);
        }
    }

    // Audio processing
    float* channels[2] = { audioBuffer.getWritePointer(0), audioBuffer.getWritePointer(1) };
    int numSamples = audioBuffer.getNumSamples();

    for (int i = 0; i < numSamples; i++)
    {
        float smp[2];
        smp[0] = channels[0][i];
        smp[1] = channels[1][i];
        DoProcess(smp);
        channels[0][i] = smp[0] * fOutVol;
        channels[1][i] = smp[1] * fOutVol;
    }
}

bool MadshiftaProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* MadshiftaProcessor::createEditor()
{
    return new MadshiftaEditor(*this);
}

void MadshiftaProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void MadshiftaProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MadshiftaProcessor();
}
