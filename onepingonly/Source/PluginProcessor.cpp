#include "PluginProcessor.h"
#include "Delay.h"
#include <limits>

// Static random generator seeded once at load time
static juce::Random& getSharedRandom()
{
    static juce::Random random(juce::Time::currentTimeMillis());
    return random;
}

// Helper for random float 0-1
inline float randomFloat()
{
    return getSharedRandom().nextFloat();
}

// Helper to check for denormals
inline bool isDenormal(float flt)
{
    return std::fpclassify(flt) == FP_SUBNORMAL;
}

OnePingOnlyProcessor::OnePingOnlyProcessor()
    : AudioProcessor(BusesProperties()
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout()),
      rng(static_cast<unsigned int>(std::time(nullptr)))
{
    delayL = std::make_unique<CDelay>(static_cast<int>(44100 * MAX_DELAY_SECONDS));
    delayR = std::make_unique<CDelay>(static_cast<int>(44100 * MAX_DELAY_SECONDS));

    // Initialize filter coefficients
    for (int i = 0; i < NUM_PINGS; i++)
    {
        beta[i] = 0.99f;
        gamma[i] = -0.99f;
        alpha[i] = 1.9701f;
        d1[i] = d2[i] = 0.0f;
    }

    // Get global parameter pointers
    delayParam = apvts.getRawParameterValue(DELAY_ID);
    feedParam = apvts.getRawParameterValue(FEED_ID);
    masterParam = apvts.getRawParameterValue(MASTER_ID);

    // Get per-ping parameter pointers AND set random values
    for (int i = 0; i < NUM_PINGS; i++)
    {
        freqParams[i] = apvts.getRawParameterValue(getFreqParamID(i));
        durationParams[i] = apvts.getRawParameterValue(getDurationParamID(i));
        ampParams[i] = apvts.getRawParameterValue(getAmpParamID(i));
        balParams[i] = apvts.getRawParameterValue(getBalParamID(i));
        noiseParams[i] = apvts.getRawParameterValue(getNoiseParamID(i));
        distParams[i] = apvts.getRawParameterValue(getDistParamID(i));

        // Set random values on the actual parameters (like original PingSynthProgram)
        apvts.getParameter(getFreqParamID(i))->setValueNotifyingHost(randomFloat());
        apvts.getParameter(getDurationParamID(i))->setValueNotifyingHost(0.25f * randomFloat());
        apvts.getParameter(getAmpParamID(i))->setValueNotifyingHost(1.0f);
        apvts.getParameter(getBalParamID(i))->setValueNotifyingHost(randomFloat());
        apvts.getParameter(getNoiseParamID(i))->setValueNotifyingHost(0.0f);
        apvts.getParameter(getDistParamID(i))->setValueNotifyingHost(0.5f * randomFloat());
    }

    // Set global parameter values
    apvts.getParameter(DELAY_ID)->setValueNotifyingHost(0.25f);
    apvts.getParameter(FEED_ID)->setValueNotifyingHost(0.6f);
    apvts.getParameter(MASTER_ID)->setValueNotifyingHost(1.0f);

    // Now read them back and update DSP
    for (int i = 0; i < NUM_PINGS; i++)
    {
        fFreq[i] = freqParams[i]->load();
        fDuration[i] = durationParams[i]->load();
        fAmp[i] = ampParams[i]->load();
        fBal[i] = balParams[i]->load();
        fNoise[i] = noiseParams[i]->load();
        fDist[i] = distParams[i]->load();

        setFreq(i, fFreq[i]);
        setDuration(i, fDuration[i]);
        setAmp(i, fAmp[i]);
        setBalance(i, fBal[i]);
        setNoise(i, fNoise[i]);
        setDistortion(i, fDist[i]);
    }

    fDelay = delayParam->load();
    fFeed = feedParam->load();
    fMaster = masterParam->load();
    setDelay(fDelay);
    setFeed(fFeed);

    clearFilterStates();
}

OnePingOnlyProcessor::~OnePingOnlyProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout OnePingOnlyProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Per-ping parameters (128 pings x 6 params = 768 params)
    for (int i = 0; i < NUM_PINGS; i++)
    {
        // Frequency: displays as 0-2000 Hz
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(getFreqParamID(i), 1),
            "Freq " + juce::String(i + 1),
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
            randomFloat(),
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) {
                return juce::String(value * 2000.0f, 0) + " Hz";
            },
            nullptr));

        // Duration: displays as 0-3 seconds
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(getDurationParamID(i), 1),
            "Dur " + juce::String(i + 1),
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
            0.25f * randomFloat(),
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) {
                return juce::String(value * 3.0f, 2) + " s";
            },
            nullptr));

        // Amplitude
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(getAmpParamID(i), 1),
            "Amp " + juce::String(i + 1),
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
            1.0f));

        // Balance (pan)
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(getBalParamID(i), 1),
            "Bal " + juce::String(i + 1),
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
            randomFloat(),
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) {
                if (value < 0.45f)
                    return juce::String(static_cast<int>((0.5f - value) * 200)) + "% L";
                else if (value > 0.55f)
                    return juce::String(static_cast<int>((value - 0.5f) * 200)) + "% R";
                else
                    return juce::String("C");
            },
            nullptr));

        // Noise
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(getNoiseParamID(i), 1),
            "Noise " + juce::String(i + 1),
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
            0.0f));

        // Distortion
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(getDistParamID(i), 1),
            "Dist " + juce::String(i + 1),
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
            0.5f * randomFloat()));
    }

    // Global parameters (3)
    // Delay: 0-3 seconds
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(DELAY_ID, 1),
        "Delay",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.25f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(value * 3.0f, 2) + " s";
        },
        nullptr));

    // Feedback: 0-100%
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FEED_ID, 1),
        "Feedback",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.6f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(static_cast<int>(value * 100.0f)) + "%";
        },
        nullptr));

    // Master volume (display as dB)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(MASTER_ID, 1),
        "Master",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        1.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            if (value < 0.0001f)
                return juce::String("-inf dB");
            float dB = 20.0f * std::log10(value);
            return juce::String(dB, 1) + " dB";
        },
        nullptr));

    return { params.begin(), params.end() };
}

const juce::String OnePingOnlyProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OnePingOnlyProcessor::acceptsMidi() const
{
    return true;
}

bool OnePingOnlyProcessor::producesMidi() const
{
    return false;
}

bool OnePingOnlyProcessor::isMidiEffect() const
{
    return false;
}

double OnePingOnlyProcessor::getTailLengthSeconds() const
{
    return MAX_DELAY_SECONDS + 3.0; // Delay plus ring time
}

int OnePingOnlyProcessor::getNumPrograms()
{
    return 1;
}

int OnePingOnlyProcessor::getCurrentProgram()
{
    return 0;
}

void OnePingOnlyProcessor::setCurrentProgram(int /*index*/)
{
}

const juce::String OnePingOnlyProcessor::getProgramName(int /*index*/)
{
    return {};
}

void OnePingOnlyProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/)
{
}

void OnePingOnlyProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = static_cast<float>(sampleRate);

    delayL->setMaxDelay(static_cast<int>(sampleRate * MAX_DELAY_SECONDS));
    delayR->setMaxDelay(static_cast<int>(sampleRate * MAX_DELAY_SECONDS));

    // Update delay time for new sample rate
    setDelay(fDelay);

    // Recalculate filter coefficients for new sample rate
    for (int i = 0; i < NUM_PINGS; i++)
    {
        setFreq(i, fFreq[i]);
        setDuration(i, fDuration[i]);
    }

    clearFilterStates();
}

void OnePingOnlyProcessor::releaseResources()
{
    clearFilterStates();
}

void OnePingOnlyProcessor::clearFilterStates()
{
    for (int i = 0; i < NUM_PINGS; i++)
    {
        in[i] = in_1[i] = in_2[i] = out_1[i] = out_2[i] = 0.0f;
    }
    delayL->killBuffer();
    delayR->killBuffer();
}

bool OnePingOnlyProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Synth: no input, stereo output
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void OnePingOnlyProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                         juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Update parameters from APVTS (always update - DSP methods are lightweight)
    for (int i = 0; i < NUM_PINGS; i++)
    {
        fFreq[i] = freqParams[i]->load();
        fDuration[i] = durationParams[i]->load();
        fAmp[i] = ampParams[i]->load();
        fBal[i] = balParams[i]->load();
        fNoise[i] = noiseParams[i]->load();
        fDist[i] = distParams[i]->load();

        setFreq(i, fFreq[i]);
        setDuration(i, fDuration[i]);
        setAmp(i, fAmp[i]);
        setBalance(i, fBal[i]);
        setNoise(i, fNoise[i]);
        setDistortion(i, fDist[i]);
    }

    fDelay = delayParam->load();
    fFeed = feedParam->load();
    fMaster = masterParam->load();

    setDelay(fDelay);
    setFeed(fFeed);

    // Process MIDI events - extract note-ons
    numNotes = 0;
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            int note = msg.getNoteNumber() - LOW_NOTE;
            if (note >= 0 && note < NUM_PINGS && numNotes < 200)
            {
                notes[numNotes].note = note;
                notes[numNotes].velocity = msg.getFloatVelocity();
                notes[numNotes].time = metadata.samplePosition;
                numNotes++;
            }
        }
    }

    // Get output pointers
    float* outL = buffer.getWritePointer(0);
    float* outR = buffer.getWritePointer(1);

    int sampleFrames = buffer.getNumSamples();
    NoteData* pNotes = notes;

    float out, o, oL, oR, noise;
    int n = 0;
    int index = 0;
    bool moreThanOne = false;

    if (numNotes == 0)
        goto NoMoreNotes;

    while (n < sampleFrames)
    {
        noise = noiseDist(rng);

        if (n == pNotes->time)
        {
            // Process note
            index = pNotes->note;
            in[index] = g[index] * pNotes->velocity;
            pNotes++;
            numNotes--;

            // Check for other notes at this timeframe
            while (numNotes > 0 && pNotes->time == n)
            {
                index = pNotes->note;
                in[index] = g[index] * pNotes->velocity;
                pNotes++;
                numNotes--;
                moreThanOne = true;
            }

            oL = oR = o = 0.0f;

            // Main filter loop
            for (int i = 0; i < NUM_PINGS; i++)
            {
                out = beta[i] * (in[i] - out_2[i]) + alpha[i] * (in_1[i] - out_1[i]) + in_2[i];
                out_2[i] = out_1[i];
                out_1[i] = out;
                in_2[i] = in_1[i];
                in_1[i] = in[i];

                o = (out - in[i]) * (1.0f + noiseAmount[i] * noise);
                o *= d2[i] / (1.0f + d1[i] * o * o);
                oL += bal[i] * o;
                oR += baltmp[i] * o;
            }

            *outL++ = delayL->getVal(oL) * fMaster;
            *outR++ = delayR->getVal(oR) * fMaster;

            if (moreThanOne)
            {
                for (int i = 0; i < NUM_PINGS; i++)
                    in[i] = 0.0f;
                moreThanOne = false;
            }
            else
            {
                in[index] = 0.0f;
            }

            n++;

            if (numNotes == 0)
                goto NoMoreNotes;
        }
        else
        {
            oL = oR = o = 0.0f;

            for (int i = 0; i < NUM_PINGS; i++)
            {
                out = -beta[i] * out_2[i] + alpha[i] * (in_1[i] - out_1[i]) + in_2[i];
                out_2[i] = out_1[i];
                out_1[i] = out;
                in_2[i] = in_1[i];
                in_1[i] = 0.0f;

                o = out * (1.0f + noiseAmount[i] * noise);
                o *= d2[i] / (1.0f + d1[i] * o * o);
                oL += bal[i] * o;
                oR += baltmp[i] * o;
            }

            *outL++ = delayL->getVal(oL) * fMaster;
            *outR++ = delayR->getVal(oR) * fMaster;

            n++;
        }
    }

    goto end;

NoMoreNotes:
    sampleFrames -= n;

    if (sampleFrames > 2)
    {
        noise = noiseDist(rng);
        oL = oR = o = 0.0f;

        for (int i = 0; i < NUM_PINGS; i++)
        {
            out = -beta[i] * out_2[i] + alpha[i] * (in_1[i] - out_1[i]) + in_2[i];
            out_2[i] = out_1[i];
            out_1[i] = out;
            in_2[i] = in_1[i];
            in_1[i] = 0.0f;

            o = out * (1.0f + noiseAmount[i] * noise);
            o *= d2[i] / (1.0f + d1[i] * o * o);
            oL += bal[i] * o;
            oR += baltmp[i] * o;
        }

        *outL++ = delayL->getVal(oL) * fMaster;
        *outR++ = delayR->getVal(oR) * fMaster;

        noise = noiseDist(rng);
        oL = oR = o = 0.0f;

        for (int i = 0; i < NUM_PINGS; i++)
        {
            out = -beta[i] * out_2[i] - alpha[i] * out_1[i] + in_2[i];
            out_2[i] = out_1[i];
            out_1[i] = out;
            in_2[i] = 0.0f;

            o = out * (1.0f + noiseAmount[i] * noise);
            o *= d2[i] / (1.0f + d1[i] * o * o);
            oL += bal[i] * o;
            oR += baltmp[i] * o;
        }

        *outL++ = delayL->getVal(oL) * fMaster;
        *outR++ = delayR->getVal(oR) * fMaster;

        sampleFrames -= 2;

        while (sampleFrames--)
        {
            noise = noiseDist(rng);
            oL = oR = o = 0.0f;

            for (int i = 0; i < NUM_PINGS; i++)
            {
                out = -beta[i] * out_2[i] - alpha[i] * out_1[i];
                out_2[i] = out_1[i];
                out_1[i] = out;

                o = out * (1.0f + noiseAmount[i] * noise);
                o *= d2[i] / (1.0f + d1[i] * o * o);
                oL += bal[i] * o;
                oR += (1.0f - bal[i]) * o;
            }

            *outL++ = delayL->getVal(oL) * fMaster;
            *outR++ = delayR->getVal(oR) * fMaster;
        }
    }
    else
    {
        while (sampleFrames--)
        {
            noise = noiseDist(rng);
            oL = oR = o = 0.0f;

            for (int i = 0; i < NUM_PINGS; i++)
            {
                out = -beta[i] * out_2[i] + alpha[i] * (in_1[i] - out_1[i]) + in_2[i];
                out_2[i] = out_1[i];
                out_1[i] = out;
                in_2[i] = in_1[i];
                in_1[i] = 0.0f;

                o = out * (1.0f + noiseAmount[i] * noise);
                o *= d2[i] / (1.0f + d1[i] * o * o);
                oL += bal[i] * o;
                oR += baltmp[i] * o;
            }

            *outL++ = delayL->getVal(oL) * fMaster;
            *outR++ = delayR->getVal(oR) * fMaster;
        }
    }

end:
    notes[0].note = -1;

    // Avoid denormals
    for (int i = 0; i < NUM_PINGS; i++)
    {
        if (isDenormal(out_1[i]))
            out_1[i] = 0.0f;
        if (isDenormal(out_2[i]))
            out_2[i] = 0.0f;
    }
}

// DSP parameter methods (from original)
void OnePingOnlyProcessor::setFreq(int index, float freq)
{
    if (freq < 0.001f)
        freq = 0.001f;

    gamma[index] = -std::cos((2.0f * 3.1415926535f * 2000.0f * freq) / currentSampleRate);
    alpha[index] = gamma[index] * (1.0f + beta[index]);
}

void OnePingOnlyProcessor::setAmp(int index, float ampVal)
{
    amp[index] = ampVal;
    g[index] = amp[index] * 0.4f / (1.0f - beta[index]);
}

void OnePingOnlyProcessor::setDuration(int index, float duration)
{
    duration *= 3.0f * (currentSampleRate / 44100.0f);
    beta[index] = (duration * 8000.0f - 1.0f) / (duration * 8000.0f + 1.0f);
    alpha[index] = gamma[index] * (1.0f + beta[index]);
    g[index] = amp[index] * 0.4f / (1.0f - beta[index]);
}

void OnePingOnlyProcessor::setBalance(int index, float balance)
{
    bal[index] = balance;
    baltmp[index] = 1.0f - balance;
}

void OnePingOnlyProcessor::setNoise(int index, float n)
{
    noiseAmount[index] = n;
}

void OnePingOnlyProcessor::setDistortion(int index, float dist)
{
    dist *= 10.0f;

    d1[index] = dist;

    if (dist > 1.0f)
        d2[index] = (1.0f + dist) * (1.0f - 0.8f * (dist - 1.0f) / 19.0f);
    else
        d2[index] = 1.0f + dist;
}

void OnePingOnlyProcessor::setDelay(float delay)
{
    int d = static_cast<int>(delay * currentSampleRate * MAX_DELAY_SECONDS);
    delayL->setDelay(d);
    delayR->setDelay(d);
}

void OnePingOnlyProcessor::setFeed(float feed)
{
    delayL->setFeed(feed);
    delayR->setFeed(feed);
}

bool OnePingOnlyProcessor::hasEditor() const
{
    return false;
}

juce::AudioProcessorEditor* OnePingOnlyProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

void OnePingOnlyProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void OnePingOnlyProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));

        // Update all DSP state from restored parameters
        for (int i = 0; i < NUM_PINGS; i++)
        {
            fFreq[i] = freqParams[i]->load();
            fDuration[i] = durationParams[i]->load();
            fAmp[i] = ampParams[i]->load();
            fBal[i] = balParams[i]->load();
            fNoise[i] = noiseParams[i]->load();
            fDist[i] = distParams[i]->load();

            setFreq(i, fFreq[i]);
            setDuration(i, fDuration[i]);
            setAmp(i, fAmp[i]);
            setBalance(i, fBal[i]);
            setNoise(i, fNoise[i]);
            setDistortion(i, fDist[i]);
        }

        fDelay = delayParam->load();
        fFeed = feedParam->load();
        fMaster = masterParam->load();
        setDelay(fDelay);
        setFeed(fFeed);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OnePingOnlyProcessor();
}
