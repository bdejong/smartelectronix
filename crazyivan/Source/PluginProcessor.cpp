#include "PluginProcessor.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

// Helper functions from original
namespace {
    float sq(float x) { return x * x * x; }

    float clip(float x)
    {
        if (x > 1.f)
            return 1.f;
        if (x < -1.f)
            return -1.f;
        return x;
    }

    float TriToSine(float value)
    {
        float v2 = value * value;
        return value * (1.569799213f + 0.5f * (-1.279196852f + 0.139598426f * v2) * v2);
    }

    float process1(float in, short mask, bool b_xor, bool b_and, bool power)
    {
        bool sign = in < 0.f;
        in = clip(std::fabs(in));

        if (power)
            in = std::pow(in, 1.f / 3.f);

        short x = static_cast<short>(in * 32767.f);

        if (b_xor)
            x = x ^ mask;

        if (b_and)
            x = x & mask;

        x &= 0x7fff;

        float out = static_cast<float>(x) / 32767.f;

        if (power)
            out = std::pow(out, 3.f);

        return sign ? -out : out;
    }

    float randfloat(float min = 0.f, float max = 1.f)
    {
        return min + (max - min) * static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    }

    float randbool()
    {
        return (std::rand() > (RAND_MAX / 2)) ? 1.f : 0.f;
    }
}

CrazyIvanProcessor::CrazyIvanProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout()),
      Delay(1000000)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Get parameter pointers
    murderParam = apvts.getRawParameterValue(MURDER_ID);
    bitsParam = apvts.getRawParameterValue(BITS_ID);
    andParam = apvts.getRawParameterValue(AND_ID);
    xorParam = apvts.getRawParameterValue(XOR_ID);
    powerParam = apvts.getRawParameterValue(POWER_ID);
    freq1Param = apvts.getRawParameterValue(FREQ1_ID);
    dfreq1Param = apvts.getRawParameterValue(DFREQ1_ID);
    vibfreqParam = apvts.getRawParameterValue(VIBFREQ_ID);
    dvibfreqParam = apvts.getRawParameterValue(DVIBFREQ_ID);
    feed1Param = apvts.getRawParameterValue(FEED1_ID);
    freq2Param = apvts.getRawParameterValue(FREQ2_ID);
    mindelayParam = apvts.getRawParameterValue(MINDELAY_ID);
    maxdelayParam = apvts.getRawParameterValue(MAXDELAY_ID);
    distParam = apvts.getRawParameterValue(DIST_ID);
    dampParam = apvts.getRawParameterValue(DAMP_ID);
    feed2Param = apvts.getRawParameterValue(FEED2_ID);
    limiterParam = apvts.getRawParameterValue(LIMITER_ID);
    attackParam = apvts.getRawParameterValue(ATTACK_ID);
    releaseParam = apvts.getRawParameterValue(RELEASE_ID);
    ampParam = apvts.getRawParameterValue(AMP_ID);
    drywetParam = apvts.getRawParameterValue(DRYWET_ID);
    randomiseParam = apvts.getRawParameterValue(RANDOMISE_ID);
}

CrazyIvanProcessor::~CrazyIvanProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout CrazyIvanProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Helper: sq(x) = x^3
    auto sq = [](float x) { return x * x * x; };

    // Bitmurderer section
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID(MURDER_ID, 1), "Bitmurderer", false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(BITS_ID, 1), "Bitmask",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(static_cast<int>(value * 32768.0f * 0.49f));
        }, nullptr));

    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID(AND_ID, 1), "AND", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID(XOR_ID, 1), "XOR", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID(POWER_ID, 1), "Power", false));

    // Oscillator 1 section
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FREQ1_ID, 1), "Frequency 1",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [sq](float value, int) {
            return juce::String(sq(value) * 300.0f, 1) + " Hz";
        }, nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(DFREQ1_ID, 1), "Diff Frequency 1",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(value, 2) + " x freq1";
        }, nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(VIBFREQ_ID, 1), "Vib Frequency",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [sq](float value, int) {
            return juce::String(sq(value) * 10.0f, 2) + " Hz";
        }, nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(DVIBFREQ_ID, 1), "Diff Vib Frequency",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [sq](float value, int) {
            return juce::String(sq(value) * 10.0f, 2) + " Hz";
        }, nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FEED1_ID, 1), "Feedback 1",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(value * 0.99f, 2);
        }, nullptr));

    // Oscillator 2 / Flanger section
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FREQ2_ID, 1), "Frequency 2",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(value, 2) + " Hz";
        }, nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(MINDELAY_ID, 1), "Min Delay",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(value * 10.0f, 1) + " smp";
        }, nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(MAXDELAY_ID, 1), "Max Delay",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(static_cast<int>(value * 15000.0f)) + " smp";
        }, nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(DIST_ID, 1), "Distortion",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(value * 30.0f, 1);
        }, nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(DAMP_ID, 1), "Damping",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(value, 2);
        }, nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(FEED2_ID, 1), "Feedback 2",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(value * 0.99f, 2);
        }, nullptr));

    // Limiter section
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(LIMITER_ID, 1), "Limiter",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.3f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(value, 2);
        }, nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ATTACK_ID, 1), "Attack",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(value * 200.0f, 1) + " ms";
        }, nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(RELEASE_ID, 1), "Release",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(value * 200.0f, 1) + " ms";
        }, nullptr));

    // Output section
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AMP_ID, 1), "Amplify",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            float scaled = (value * value) * 2.0f;
            if (scaled < 0.0001f) return juce::String("-inf dB");
            float dB = 20.0f * std::log10(scaled);
            return juce::String(dB, 1) + " dB";
        }, nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(DRYWET_ID, 1), "Dry/Wet",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 1.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(static_cast<int>(value * 100.0f)) + "%";
        }, nullptr));

    // Randomise button
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID(RANDOMISE_ID, 1), "Randomise", false));

    return { params.begin(), params.end() };
}

const juce::String CrazyIvanProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CrazyIvanProcessor::acceptsMidi() const { return false; }
bool CrazyIvanProcessor::producesMidi() const { return false; }
bool CrazyIvanProcessor::isMidiEffect() const { return false; }
double CrazyIvanProcessor::getTailLengthSeconds() const { return 0.0; }

int CrazyIvanProcessor::getNumPrograms() { return 1; }
int CrazyIvanProcessor::getCurrentProgram() { return 0; }
void CrazyIvanProcessor::setCurrentProgram(int /*index*/) {}
const juce::String CrazyIvanProcessor::getProgramName(int /*index*/) { return {}; }
void CrazyIvanProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/) {}

void CrazyIvanProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = static_cast<float>(sampleRate);
    S.SampleRateF = currentSampleRate;
    S2.SampleRateF = currentSampleRate;
    F.T.SampleRateF = currentSampleRate;
    E.SampleRateF = currentSampleRate;

    // Reset state
    ivanOutput = 0.0f;
    F.suspend();
    LP.suspend();
    S.suspend();
    E.suspend();
    Delay.suspend();
    S2.suspend();
}

void CrazyIvanProcessor::releaseResources()
{
    ivanOutput = 0.0f;
    F.suspend();
    LP.suspend();
    S.suspend();
    E.suspend();
    Delay.suspend();
    S2.suspend();
}

bool CrazyIvanProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void CrazyIvanProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    // Check for randomise trigger (rising edge detection)
    bool randomiseNow = randomiseParam->load() > 0.5f;
    if (randomiseNow && !lastRandomiseState)
    {
        // Trigger randomise on the message thread via AsyncUpdater
        pendingRandomise.store(true);
        triggerAsyncUpdate();
    }
    lastRandomiseState = randomiseNow;

    // Get raw parameter values
    float freq1Raw = freq1Param->load();
    float dfreq1Raw = dfreq1Param->load();
    float vibfreqRaw = vibfreqParam->load();
    float dvibfreqRaw = dvibfreqParam->load();
    float feed1Raw = feed1Param->load();
    float freq2Raw = freq2Param->load();
    float mindelayRaw = mindelayParam->load();
    float maxdelayRaw = maxdelayParam->load();
    float distRaw = distParam->load();
    float dampRaw = dampParam->load();
    float feed2Raw = feed2Param->load();
    float limiterRaw = limiterParam->load();
    float attackRaw = attackParam->load();
    float releaseRaw = releaseParam->load();
    float ampRaw = ampParam->load();
    float drywetRaw = drywetParam->load();

    // Scale parameters (matching original setParameter logic)
    float scaledFreq1 = sq(freq1Raw) * 300.f;
    float scaledDFreq1 = scaledFreq1 * dfreq1Raw;
    float scaledVibfreq = sq(vibfreqRaw) * 10.f;
    float scaledDVibfreq = sq(dvibfreqRaw) * 10.f;
    float scaledFeed1 = feed1Raw * 0.99f;
    float scaledFreq2 = freq2Raw;
    float scaledMindelay = mindelayRaw * 10.f;
    float scaledMaxdelay = maxdelayRaw * 15000.f;
    float scaledDist = distRaw * 30.f;
    float scaledDamp = dampRaw;
    float scaledFeed2 = feed2Raw * 0.99f;
    float scaledLimiter = limiterRaw;
    float scaledAttack = attackRaw * 200.f;
    float scaledRelease = releaseRaw * 200.f;
    float scaledAmp = (ampRaw * ampRaw) * 2.f;
    float scaledDrywet = drywetRaw;

    // Update DSP parameters
    LP.SetParams(1.f - scaledDamp);

    float x1 = std::fabs(scaledFreq1 - scaledDFreq1);
    float x2 = std::fabs(scaledFreq1 + scaledDFreq1);
    if (x1 < 0.01f) x1 = 0.01f;
    if (x2 < 0.01f) x2 = 0.01f;
    x1 = currentSampleRate / x1;
    if (x1 > 500000.f) x1 = 500000.f;
    x2 = currentSampleRate / x2;
    if (x2 > 500000.f) x2 = 500000.f;

    S.SetParams(x1, x2);
    F.SetParams(scaledFreq2, scaledFeed2, scaledDrywet, scaledDist, scaledMindelay, scaledMaxdelay);
    E.SetParams(scaledAttack, scaledRelease);

    // Process
    bool murder = murderParam->load() > 0.5f;
    short mask = static_cast<short>(bitsParam->load() * 32768.f * 0.49f);
    bool b_and = andParam->load() > 0.5f;
    bool b_xor = xorParam->load() > 0.5f;
    bool power = powerParam->load() > 0.5f;

    float tmp1 = 1.f - scaledDrywet;
    float tmp2 = scaledDrywet * scaledAmp;
    float tmp4 = 1.f - scaledFeed1;
    float tmp3 = scaledLimiter * 0.5f;

    float* channelL = buffer.getWritePointer(0);
    float* channelR = buffer.getWritePointer(1);
    const int numSamples = buffer.getNumSamples();

    for (int i = 0; i < numSamples; i++)
    {
        float bitout;

        if (murder)
            bitout = process1(channelL[i], mask, b_xor, b_and, power);
        else
            bitout = channelL[i];

        ivanOutput = F.GetVal(Delay.GetVal(LP.GetVal(scaledFeed1 * ivanOutput + tmp4 * bitout),
                                            S.GetVal(TriToSine(S2.GetVal(scaledVibfreq)) * scaledDVibfreq)));

        float envOut = E.GetVal(ivanOutput);

        if (envOut < 1.f)
            ivanOutput *= 1.f - tmp3 * envOut;
        else
            ivanOutput *= (1.f - tmp3 * 2.f) + tmp3 / envOut;

        float output = channelL[i] * tmp1 + ivanOutput * tmp2;
        channelL[i] = output;
        channelR[i] = output;
    }

    // Denormal protection
    if (std::fabs(ivanOutput) < 1e-10f || std::fabs(ivanOutput) > 1e10f)
        ivanOutput = 0.f;
}

bool CrazyIvanProcessor::hasEditor() const { return false; }

juce::AudioProcessorEditor* CrazyIvanProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

void CrazyIvanProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void CrazyIvanProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

void CrazyIvanProcessor::handleAsyncUpdate()
{
    if (pendingRandomise.exchange(false))
    {
        randomise();
        // Reset the randomise button back to off
        if (auto* param = apvts.getParameter(RANDOMISE_ID))
            param->setValueNotifyingHost(0.0f);
    }
}

void CrazyIvanProcessor::randomise()
{
    apvts.getParameter(MURDER_ID)->setValueNotifyingHost(randbool());
    apvts.getParameter(BITS_ID)->setValueNotifyingHost(randfloat());
    apvts.getParameter(AND_ID)->setValueNotifyingHost(randbool());
    apvts.getParameter(XOR_ID)->setValueNotifyingHost(randbool());
    apvts.getParameter(POWER_ID)->setValueNotifyingHost(randbool());
    apvts.getParameter(FREQ1_ID)->setValueNotifyingHost(randfloat());
    apvts.getParameter(DFREQ1_ID)->setValueNotifyingHost(randfloat());
    apvts.getParameter(VIBFREQ_ID)->setValueNotifyingHost(randfloat());
    apvts.getParameter(DVIBFREQ_ID)->setValueNotifyingHost(randfloat());
    apvts.getParameter(FEED1_ID)->setValueNotifyingHost(randfloat());
    apvts.getParameter(FREQ2_ID)->setValueNotifyingHost(randfloat());
    apvts.getParameter(MINDELAY_ID)->setValueNotifyingHost(randfloat());
    apvts.getParameter(MAXDELAY_ID)->setValueNotifyingHost(randfloat());
    apvts.getParameter(DIST_ID)->setValueNotifyingHost(randfloat());
    apvts.getParameter(FEED2_ID)->setValueNotifyingHost(randfloat());
    apvts.getParameter(DAMP_ID)->setValueNotifyingHost(randfloat());
    apvts.getParameter(LIMITER_ID)->setValueNotifyingHost(0.3f);
    apvts.getParameter(ATTACK_ID)->setValueNotifyingHost(randfloat());
    apvts.getParameter(RELEASE_ID)->setValueNotifyingHost(randfloat());
    apvts.getParameter(AMP_ID)->setValueNotifyingHost(0.5f);
    apvts.getParameter(DRYWET_ID)->setValueNotifyingHost(1.f);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CrazyIvanProcessor();
}
