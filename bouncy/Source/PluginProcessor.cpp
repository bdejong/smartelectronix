#include "PluginProcessor.h"

BouncyProcessor::BouncyProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    delayL = std::make_unique<Bouncy>(44100.f);
    delayR = std::make_unique<Bouncy>(44100.f);

    maxDelayParam = apvts.getRawParameterValue(MAX_DELAY_ID);
    delayShapeParam = apvts.getRawParameterValue(DELAY_SHAPE_ID);
    ampShapeParam = apvts.getRawParameterValue(AMP_SHAPE_ID);
    randAmpParam = apvts.getRawParameterValue(RAND_AMP_ID);
    renewRandParam = apvts.getRawParameterValue(RENEW_RAND_ID);
}

BouncyProcessor::~BouncyProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout BouncyProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Max delay: 0-1 maps to 0-16 beats, display as beats
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(MAX_DELAY_ID, 1),
        "Max Delay",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.58f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(value * 16.0f, 2) + " beats";
        },
        nullptr));

    // Delay shape: 0-1 maps to -1 to +1
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(DELAY_SHAPE_ID, 1),
        "Delay Shape",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.42f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            float shaped = value * 2.0f - 1.0f;
            return juce::String(shaped, 2);
        },
        nullptr));

    // Amp shape: 0-1 maps to -1 to +1
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AMP_SHAPE_ID, 1),
        "Amp Shape",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.08f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            float shaped = value * 2.0f - 1.0f;
            return juce::String(shaped, 2);
        },
        nullptr));

    // Random amplitude: display as 0-1
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(RAND_AMP_ID, 1),
        "Rand Amp",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f));

    // Renew random (bool)
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(RENEW_RAND_ID, 1),
        "Renew Rand",
        false));

    return { params.begin(), params.end() };
}

const juce::String BouncyProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BouncyProcessor::acceptsMidi() const
{
    return true;
}

bool BouncyProcessor::producesMidi() const
{
    return false;
}

bool BouncyProcessor::isMidiEffect() const
{
    return false;
}

double BouncyProcessor::getTailLengthSeconds() const
{
    return MAX_DELAY_SECONDS;
}

int BouncyProcessor::getNumPrograms()
{
    return 1;
}

int BouncyProcessor::getCurrentProgram()
{
    return 0;
}

void BouncyProcessor::setCurrentProgram(int /*index*/)
{
}

const juce::String BouncyProcessor::getProgramName(int /*index*/)
{
    return {};
}

void BouncyProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/)
{
}

void BouncyProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    delayL->setSamplerate(static_cast<float>(sampleRate));
    delayR->setSamplerate(static_cast<float>(sampleRate));
    delayL->resume();
    delayR->resume();
    isDirty = true;
}

void BouncyProcessor::releaseResources()
{
    delayL->resume();
    delayR->resume();
}

bool BouncyProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void BouncyProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Handle MIDI CC messages (matching original CC numbers)
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isController())
        {
            const int cc = msg.getControllerNumber();
            const float value = msg.getControllerValue() / 127.0f;

            switch (cc)
            {
                case 73: apvts.getParameter(MAX_DELAY_ID)->setValueNotifyingHost(value); break;
                case 74: apvts.getParameter(DELAY_SHAPE_ID)->setValueNotifyingHost(value); break;
                case 75: apvts.getParameter(AMP_SHAPE_ID)->setValueNotifyingHost(value); break;
                case 76: apvts.getParameter(RAND_AMP_ID)->setValueNotifyingHost(value); break;
                case 77: apvts.getParameter(RENEW_RAND_ID)->setValueNotifyingHost(value); break;
            }
        }
    }

    // Get tempo from host
    float newBPM = 120.0f;
    if (auto* playHead = getPlayHead())
    {
        if (auto posInfo = playHead->getPosition())
        {
            if (auto bpm = posInfo->getBpm())
            {
                newBPM = static_cast<float>(*bpm);
                if (newBPM < 10.0f)
                    newBPM = 120.0f;
            }
        }
    }

    // Get parameter values
    float maxDelay = maxDelayParam->load();
    float delayShape = delayShapeParam->load();
    float ampShape = ampShapeParam->load();
    float randAmp = randAmpParam->load();
    bool renewRand = renewRandParam->load() > 0.5f;

    // Handle renew random trigger
    if (renewRand && !lastRenewRandState)
    {
        delayL->fillRand();
        delayR->fillRand();
    }
    lastRenewRandState = renewRand;

    // Clamp max delay based on tempo (from original)
    if ((maxDelay * 60.f * NUM_BEATS) / (newBPM * 5.f) > 1.f)
        maxDelay = (newBPM * 5.f) / (60.f * NUM_BEATS);

    // Calculate delay parameter
    float x = (maxDelay * 60.f * NUM_BEATS) / (newBPM * MAX_DELAY_SECONDS);
    if (x > 1.f) x = 1.f;

    // Always update DSP parameters (they're lightweight)
    delayL->setParameters(x, delayShape, ampShape, randAmp);
    delayR->setParameters(x, delayShape, ampShape, randAmp);
    currentBPM = newBPM;

    // Process audio
    float* channelL = buffer.getWritePointer(0);
    float* channelR = buffer.getWritePointer(1);

    delayL->process(channelL, channelL, static_cast<unsigned long>(buffer.getNumSamples()), true);
    delayR->process(channelR, channelR, static_cast<unsigned long>(buffer.getNumSamples()), true);
}

bool BouncyProcessor::hasEditor() const
{
    return false;  // No custom editor - use generic JUCE editor
}

juce::AudioProcessorEditor* BouncyProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

void BouncyProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BouncyProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        isDirty = true;
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BouncyProcessor();
}
