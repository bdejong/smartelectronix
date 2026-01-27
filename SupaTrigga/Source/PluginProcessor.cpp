#include "PluginProcessor.h"
#include <ctime>

SupaTriggaProcessor::SupaTriggaProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Allocate buffers
    leftBuffer = std::make_unique<float[]>(MAXSIZE);
    rightBuffer = std::make_unique<float[]>(MAXSIZE);

    for (size_t i = 0; i < MAXSIZE; i++)
    {
        leftBuffer[i] = 0.0f;
        rightBuffer[i] = 0.0f;
    }

    // Initialize sequencer
    for (int i = 0; i < MAXSLIDES; i++)
    {
        sequencer[i].offset = 0;
        sequencer[i].reverse = false;
        sequencer[i].stop = false;
        sequencer[i].silence = false;
    }

    // Get parameter pointers
    granularityParam = apvts.getRawParameterValue(GRANULARITY_ID);
    speedParam = apvts.getRawParameterValue(SPEED_ID);
    probReverseParam = apvts.getRawParameterValue(PROB_REVERSE_ID);
    probSpeedParam = apvts.getRawParameterValue(PROB_SPEED_ID);
    probRearrangeParam = apvts.getRawParameterValue(PROB_REARRANGE_ID);
    probSilenceParam = apvts.getRawParameterValue(PROB_SILENCE_ID);
    probRepeatParam = apvts.getRawParameterValue(PROB_REPEAT_ID);
    instantReverseParam = apvts.getRawParameterValue(INSTANT_REVERSE_ID);
    instantSpeedParam = apvts.getRawParameterValue(INSTANT_SPEED_ID);
    instantRepeatParam = apvts.getRawParameterValue(INSTANT_REPEAT_ID);

    randomize();
}

SupaTriggaProcessor::~SupaTriggaProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout SupaTriggaProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Granularity: displays as number of slices (1, 2, 4, 8, 16, 32, 64, 128)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(GRANULARITY_ID, 1), "Slices",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.3f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            int slices = 1 << static_cast<int>(value * (BITSLIDES + 0.5f));
            return juce::String(slices) + " slices/measure";
        },
        nullptr));

    // Speed: displays as speed multiplier
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(SPEED_ID, 1), "Slow Speed",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.25f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            float speedVal = (1.0f - value + 0.01f) * 4.0f;
            return juce::String(speedVal, 2) + "x";
        },
        nullptr));

    // Probability parameters: display as percentage
    auto probToString = [](float value, int) {
        return juce::String(static_cast<int>(value * 100.0f)) + "%";
    };

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PROB_REVERSE_ID, 1), "Reverse Prob",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.15f, juce::String(), juce::AudioProcessorParameter::genericParameter,
        probToString, nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PROB_SPEED_ID, 1), "Slow Prob",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.05f, juce::String(), juce::AudioProcessorParameter::genericParameter,
        probToString, nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PROB_REARRANGE_ID, 1), "Rearrange Prob",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.95f, juce::String(), juce::AudioProcessorParameter::genericParameter,
        probToString, nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PROB_SILENCE_ID, 1), "Silence Prob",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f, juce::String(), juce::AudioProcessorParameter::genericParameter,
        probToString, nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PROB_REPEAT_ID, 1), "Repeat Prob",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.4f, juce::String(), juce::AudioProcessorParameter::genericParameter,
        probToString, nullptr));

    // Instant toggles
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(INSTANT_REVERSE_ID, 1), "Instant Reverse", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(INSTANT_SPEED_ID, 1), "Instant Slow", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(INSTANT_REPEAT_ID, 1), "Instant Repeat", false));

    return { params.begin(), params.end() };
}

const juce::String SupaTriggaProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SupaTriggaProcessor::acceptsMidi() const { return false; }
bool SupaTriggaProcessor::producesMidi() const { return false; }
bool SupaTriggaProcessor::isMidiEffect() const { return false; }
double SupaTriggaProcessor::getTailLengthSeconds() const { return 0.0; }

int SupaTriggaProcessor::getNumPrograms() { return 1; }
int SupaTriggaProcessor::getCurrentProgram() { return 0; }
void SupaTriggaProcessor::setCurrentProgram(int /*index*/) {}
const juce::String SupaTriggaProcessor::getProgramName(int /*index*/) { return {}; }
void SupaTriggaProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/) {}

void SupaTriggaProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = static_cast<float>(sampleRate);
    fadeCoeff = std::exp(std::log(0.01f) / FADETIME);

    // Reset state
    positionInMeasure = 0;
    previousSliceIndex = 0xffffffff;
    granularityMask = 0;
    granularity = 0;
    gain = 0.0f;
    speed = 0.0f;
    first = true;
    wasPlaying = false;

    // Clear buffers
    for (size_t i = 0; i < MAXSIZE; i++)
    {
        leftBuffer[i] = 0.0f;
        rightBuffer[i] = 0.0f;
    }

    randomize();
}

void SupaTriggaProcessor::releaseResources()
{
}

bool SupaTriggaProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void SupaTriggaProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    auto* in1 = buffer.getReadPointer(0);
    auto* in2 = buffer.getReadPointer(1);
    auto* out1 = buffer.getWritePointer(0);
    auto* out2 = buffer.getWritePointer(1);
    const int sampleFrames = buffer.getNumSamples();

    // Get transport info from host
    auto* playHead = getPlayHead();
    if (playHead == nullptr)
    {
        // No playhead, pass through
        for (int i = 0; i < sampleFrames; i++)
        {
            out1[i] = in1[i];
            out2[i] = in2[i];
        }
        return;
    }

    auto posInfo = playHead->getPosition();
    if (!posInfo.hasValue())
    {
        // No position info, pass through
        for (int i = 0; i < sampleFrames; i++)
        {
            out1[i] = in1[i];
            out2[i] = in2[i];
        }
        return;
    }

    // Check if we have all required info
    bool isPlaying = posInfo->getIsPlaying();
    auto bpmOpt = posInfo->getBpm();
    auto timeSigOpt = posInfo->getTimeSignature();
    auto ppqPosOpt = posInfo->getPpqPosition();
    auto barPosOpt = posInfo->getPpqPositionOfLastBarStart();

    if (!isPlaying || !bpmOpt.hasValue() || !timeSigOpt.hasValue() || !ppqPosOpt.hasValue())
    {
        positionInMeasure = 0xffffffff;
        for (int i = 0; i < sampleFrames; i++)
        {
            out1[i] = in1[i];
            out2[i] = in2[i];
        }
        wasPlaying = isPlaying;
        return;
    }

    double tempo = *bpmOpt;
    if (tempo < 20.0)
    {
        // Ridiculous tempo
        for (int i = 0; i < sampleFrames; i++)
        {
            out1[i] = in1[i];
            out2[i] = in2[i];
        }
        return;
    }

    auto timeSig = *timeSigOpt;
    double ppqPos = *ppqPosOpt;
    double barStartPos = barPosOpt.hasValue() ? *barPosOpt : ppqPos;

    // Calculate samples in measure
    double tempoBPS = tempo / 60.0;
    double numSamplesInBeat = currentSampleRate / tempoBPS;
    double samplesInMeasureDouble = std::ceil(numSamplesInBeat * timeSig.numerator / (timeSig.denominator / 4.0));
    unsigned long samplesInMeasure = static_cast<unsigned long>(samplesInMeasureDouble);

    // Detect transport changes
    bool playbackChanged = (isPlaying != wasPlaying);
    wasPlaying = isPlaying;

    // Calculate distance to next bar
    double beatsPerBar = static_cast<double>(timeSig.numerator);
    double distanceToNextBarPPQ;
    if (std::fabs(barStartPos - ppqPos) < 1e-10)
        distanceToNextBarPPQ = 0.0;
    else
        distanceToNextBarPPQ = barStartPos + beatsPerBar - ppqPos;

    while (distanceToNextBarPPQ < 0.0)
        distanceToNextBarPPQ += beatsPerBar;
    while (distanceToNextBarPPQ >= beatsPerBar)
        distanceToNextBarPPQ -= beatsPerBar;

    double numSamplesToNextBar = (distanceToNextBarPPQ * currentSampleRate * 60.0) / tempo;
    if (numSamplesToNextBar < 0.0)
        numSamplesToNextBar = 0.0;

    unsigned long samplesToNextBar = static_cast<unsigned long>(std::ceil(numSamplesToNextBar));

    // If playback changed, recalculate position
    if (playbackChanged)
    {
        positionInMeasure = static_cast<unsigned long>(std::floor(samplesInMeasureDouble - numSamplesToNextBar));
        if (positionInMeasure >= samplesInMeasure)
            positionInMeasure = samplesInMeasure - 1;
    }

    // Get parameter values
    float granularityRaw = granularityParam->load();
    float speedRaw = speedParam->load();

    unsigned long granularityTmp = static_cast<unsigned long>(granularityRaw * (BITSLIDES + 0.5f));
    unsigned long granularityMaskTmp = 0xffffffff << (BITSLIDES - granularityTmp);

    float speedDiff = std::exp(std::log(0.01f) / (((1.0f - speedRaw) + 0.01f) * currentSampleRate * 4.0f));

    for (int i = 0; i < sampleFrames; i++)
    {
        // Reset position at bar border
        if (static_cast<unsigned long>(i) == samplesToNextBar)
            positionInMeasure = 0;

        // Store input in buffer
        if (positionInMeasure < MAXSIZE)
        {
            leftBuffer[positionInMeasure] = in1[i];
            rightBuffer[positionInMeasure] = in2[i];
        }

        unsigned long sliceIndex = ((positionInMeasure * MAXSLIDES) / samplesInMeasure) & granularityMask;

        if (instantRepeat && sliceIndex != 0)
            sequencer[sliceIndex].offset = (sequencer[(sliceIndex - 1) & granularityMaskTmp].offset +
                                            (sliceIndex - ((sliceIndex - 1) & granularityMaskTmp))) & granularityMask;

        unsigned long displacement = sequencer[sliceIndex].offset & granularityMask;

        if (granularityMaskTmp != granularityMask)
        {
            unsigned long sliceIndexTmp = ((positionInMeasure * MAXSLIDES) / samplesInMeasure) & granularityMaskTmp;

            if ((granularityTmp < granularity && sliceIndex == 0) ||
                (granularityTmp > granularity && sliceIndexTmp == 0) ||
                (sliceIndex == 0 && sliceIndexTmp == 0))
            {
                granularityMask = granularityMaskTmp;
                granularity = granularityTmp;
                sliceIndex = sliceIndexTmp;
                displacement = sequencer[sliceIndex].offset & granularityMask;
            }
        }

        if (sliceIndex != previousSliceIndex)
        {
            instantRepeat = instantRepeatParam->load() > 0.5f;
            instantReverse = instantReverseParam->load() > 0.5f;
            instantSlow = instantSpeedParam->load() > 0.5f;
            previousSliceIndex = sliceIndex;
        }

        // Gain calculation
        {
            unsigned long sliceIndexFar = ((((positionInMeasure + FADETIME) % samplesInMeasure) * MAXSLIDES) / samplesInMeasure) & granularityMask;
            unsigned long displacementFar = sequencer[sliceIndexFar].offset & granularityMask;
            bool reverseFar = sequencer[sliceIndexFar].reverse;

            float targetGain = 1.0f;

            if (sequencer[sliceIndex].silence)
                targetGain = 0.0f;

            if (displacementFar != displacement ||
                positionInMeasure + FADETIME > samplesInMeasure ||
                reverseFar != sequencer[sliceIndex].reverse)
                targetGain = 0.0f;

            gain = fadeCoeff * gain + (1.0f - fadeCoeff) * targetGain;
        }

        if ((sequencer[sliceIndex].reverse || instantReverse) && displacement != 0)
        {
            if (!(sequencer[sliceIndex].stop || instantSlow))
            {
                unsigned long sliceSize = samplesInMeasure >> granularity;
                unsigned long sliceStart = (positionInMeasure / sliceSize) * sliceSize;
                unsigned long sliceDiff = positionInMeasure - sliceStart;
                unsigned long sliceEnd = (sliceStart + sliceSize) - sliceDiff;
                unsigned long difference = (displacement * samplesInMeasure) / MAXSLIDES;
                unsigned long bufferIndex = difference <= sliceEnd ? sliceEnd - difference : 0;

                if (bufferIndex < MAXSIZE)
                {
                    out1[i] = leftBuffer[bufferIndex] * gain;
                    out2[i] = rightBuffer[bufferIndex] * gain;
                }
                first = true;
            }
            else
            {
                if (first)
                {
                    unsigned long sliceSize = samplesInMeasure >> granularity;
                    unsigned long sliceStart = (positionInMeasure / sliceSize) * sliceSize;
                    unsigned long sliceDiff = positionInMeasure - sliceStart;
                    unsigned long sliceEnd = (sliceStart + sliceSize) - sliceDiff;
                    unsigned long difference = (displacement * samplesInMeasure) / MAXSLIDES;

                    position = static_cast<float>(difference <= sliceEnd ? sliceEnd - difference : 0);
                    speed = 1.0f / speedDiff;
                    first = false;
                }

                speed *= speedDiff;
                position -= speed;

                if (position < 0.0f)
                    position = 0.0f;

                unsigned long bufferIndex = static_cast<unsigned long>(std::floor(position));
                float alpha = position - bufferIndex;

                if (bufferIndex < MAXSIZE - 3)
                {
                    out1[i] = hermiteInverse(leftBuffer.get(), bufferIndex, alpha) * gain;
                    out2[i] = hermiteInverse(rightBuffer.get(), bufferIndex, alpha) * gain;
                }
            }
        }
        else
        {
            if (!(sequencer[sliceIndex].stop || instantSlow) || displacement == 0)
            {
                unsigned long difference = (displacement * samplesInMeasure) / MAXSLIDES;
                unsigned long bufferIndex = positionInMeasure > difference ? positionInMeasure - difference : 0;

                if (bufferIndex < MAXSIZE)
                {
                    out1[i] = leftBuffer[bufferIndex] * gain;
                    out2[i] = rightBuffer[bufferIndex] * gain;
                }
                first = true;
            }
            else
            {
                if (first)
                {
                    position = static_cast<float>(positionInMeasure - (displacement * samplesInMeasure) / MAXSLIDES);
                    speed = 1.0f / speedDiff;
                    first = false;
                }

                speed *= speedDiff;
                position += speed;

                unsigned long bufferIndex = static_cast<unsigned long>(std::floor(position));
                float alpha = position - bufferIndex;

                if (bufferIndex < MAXSIZE - 3)
                {
                    out1[i] = hermiteInverse(leftBuffer.get(), bufferIndex, alpha) * gain;
                    out2[i] = hermiteInverse(rightBuffer.get(), bufferIndex, alpha) * gain;
                }
            }
        }

        positionInMeasure++;

        if (positionInMeasure >= samplesInMeasure)
        {
            randomize();
            positionInMeasure = 0;
        }

        if (std::fabs(gain) < 1e-10f)
            gain = 0.0f;
    }
}

void SupaTriggaProcessor::randomize()
{
    float granularityRaw = granularityParam->load();
    float probRearrangeRaw = probRearrangeParam->load();
    float probRepeatRaw = probRepeatParam->load();
    float probReverseRaw = probReverseParam->load();
    float probSpeedRaw = probSpeedParam->load();
    float probSilenceRaw = probSilenceParam->load();

    unsigned long granularityTmp = static_cast<unsigned long>(granularityRaw * (BITSLIDES + 0.5f));
    unsigned long granularityMaskTmp = 0xffffffff << (BITSLIDES - granularityTmp);

    for (unsigned long i = 0; i < MAXSLIDES; i++)
    {
        if ((std::rand() % 100) < static_cast<int>(probRearrangeRaw * 101.0f))
        {
            if ((std::rand() % 100) < static_cast<int>(probRepeatRaw * 101.0f))
            {
                if (i != 0)
                    sequencer[i].offset = (sequencer[(i - 1) & granularityMaskTmp].offset +
                                           (i - ((i - 1) & granularityMaskTmp))) & granularityMask;
                else
                    sequencer[i].offset = 0;
            }
            else
            {
                sequencer[i].offset = (i * static_cast<unsigned long>(std::rand() % MAXSLIDES)) / MAXSLIDES;
            }
        }
        else
        {
            sequencer[i].offset = 0;
        }

        if ((std::rand() % 100) < static_cast<int>(probReverseRaw * 100.0f) && sequencer[i].offset > 0)
            sequencer[i].reverse = true;
        else
            sequencer[i].reverse = false;

        if ((std::rand() % 100) < static_cast<int>(probSpeedRaw * 101.0f))
            sequencer[i].stop = true;
        else
            sequencer[i].stop = false;

        if ((std::rand() % 100) < static_cast<int>(probSilenceRaw * 101.0f))
            sequencer[i].silence = true;
        else
            sequencer[i].silence = false;
    }
}

bool SupaTriggaProcessor::hasEditor() const { return false; }

juce::AudioProcessorEditor* SupaTriggaProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

void SupaTriggaProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SupaTriggaProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SupaTriggaProcessor();
}
