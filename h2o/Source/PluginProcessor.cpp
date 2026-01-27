#include "PluginProcessor.h"
#include "PluginEditor.h"

H2OProcessor::H2OProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    compressor = std::make_unique<CCompressor>(44100.f);

    preampParam = apvts.getRawParameterValue(PREAMP_ID);
    attackParam = apvts.getRawParameterValue(ATTACK_ID);
    releaseParam = apvts.getRawParameterValue(RELEASE_ID);
    amountParam = apvts.getRawParameterValue(AMOUNT_ID);
    postampParam = apvts.getRawParameterValue(POSTAMP_ID);
    saturateParam = apvts.getRawParameterValue(SATURATE_ID);

    // Set initial parameter values to match original defaults
    // Original: fPreAmp = 0.5f, fAttack = 0.3f, fRelease = 0.1f,
    //           fAmount = 0.6f, fPostAmp = 0.6f, fSaturate = 1.f
}

H2OProcessor::~H2OProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout H2OProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PREAMP_ID, 1),
        "Input Gain",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ATTACK_ID, 1),
        "Attack",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        0.3f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(RELEASE_ID, 1),
        "Release",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        0.1f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(AMOUNT_ID, 1),
        "Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        0.6f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(POSTAMP_ID, 1),
        "Output Gain",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        0.6f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(SATURATE_ID, 1),
        "Saturate",
        true));

    return { params.begin(), params.end() };
}

const juce::String H2OProcessor::getName() const
{
    return JucePlugin_Name;
}

bool H2OProcessor::acceptsMidi() const
{
    return false;
}

bool H2OProcessor::producesMidi() const
{
    return false;
}

bool H2OProcessor::isMidiEffect() const
{
    return false;
}

double H2OProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int H2OProcessor::getNumPrograms()
{
    return 1;
}

int H2OProcessor::getCurrentProgram()
{
    return 0;
}

void H2OProcessor::setCurrentProgram(int /*index*/)
{
}

const juce::String H2OProcessor::getProgramName(int /*index*/)
{
    return {};
}

void H2OProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/)
{
}

void H2OProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    compressor->setSamplerate(static_cast<float>(sampleRate));
    compressor->Suspend();
}

void H2OProcessor::releaseResources()
{
    compressor->Suspend();
}

bool H2OProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void H2OProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                 juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    // Update compressor parameters from APVTS
    compressor->setPreamp(preampParam->load());
    compressor->setAttack(attackParam->load());
    compressor->setRelease(releaseParam->load());
    compressor->setAmount(amountParam->load());
    compressor->setPostamp(postampParam->load());
    compressor->setSaturate(saturateParam->load());

    // Process audio
    float* channels[2] = { buffer.getWritePointer(0), buffer.getWritePointer(1) };
    compressor->processReplacing(channels, channels, buffer.getNumSamples());
}

bool H2OProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* H2OProcessor::createEditor()
{
    return new H2OEditor(*this);
}

void H2OProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void H2OProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new H2OProcessor();
}
