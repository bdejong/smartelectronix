#include "PluginProcessor.h"
#include "PluginEditor.h"

BitmurdererProcessor::BitmurdererProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    for (int i = 0; i < 15; ++i)
        bitParams[i] = apvts.getRawParameterValue("bit" + juce::String(i));

    andParam = apvts.getRawParameterValue("andMode");
    orParam = apvts.getRawParameterValue("orMode");
    sigmoidParam = apvts.getRawParameterValue("sigmoid");
}

BitmurdererProcessor::~BitmurdererProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout BitmurdererProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    for (int i = 0; i < 15; ++i)
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID("bit" + juce::String(i), 1),
            "Bit " + juce::String(i), false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("andMode", 1), "AND", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("orMode", 1), "OR", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("sigmoid", 1), "Sigmoid", false));

    return { params.begin(), params.end() };
}

void BitmurdererProcessor::prepareToPlay(double, int) {}
void BitmurdererProcessor::releaseResources() {}

void BitmurdererProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Build mask from 15 bit parameters
    unsigned short tmp = 0;
    for (int i = 0; i < 15; ++i)
        tmp = (tmp << 1) + (bitParams[i]->load() > 0.5f ? 1 : 0);
    tmp |= 0x8000;
    short mask = static_cast<short>(tmp);

    bool orFlag = orParam->load() > 0.5f;
    bool andFlag = andParam->load() > 0.5f;
    bool sig = sigmoidParam->load() > 0.5f;

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float in = channelData[i];
            bool sign = in < 0.0f;
            in = std::min(std::abs(in), 1.0f);

            if (sig)
                in = std::pow(in, 1.0f / 3.0f);

            short x = static_cast<short>(in * 32767.0f);

            if (orFlag)
                x = x ^ mask;
            if (andFlag)
                x = x & mask;

            x &= 0x7FFF;

            float out = static_cast<float>(x) / 32767.0f;

            if (sig)
                out = out * out * out;

            channelData[i] = sign ? -out : out;
        }
    }
}

juce::AudioProcessorEditor* BitmurdererProcessor::createEditor()
{
    return new BitmurdererEditor(*this);
}

void BitmurdererProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BitmurdererProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BitmurdererProcessor();
}
