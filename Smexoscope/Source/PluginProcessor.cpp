#include "PluginProcessor.h"
#include "PluginEditor.h"

SmexoscopeProcessor::SmexoscopeProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    triggerSpeedParam = apvts.getRawParameterValue(PARAM_TRIGGER_SPEED);
    triggerTypeParam  = apvts.getRawParameterValue(PARAM_TRIGGER_TYPE);
    triggerLevelParam = apvts.getRawParameterValue(PARAM_TRIGGER_LEVEL);
    triggerLimitParam = apvts.getRawParameterValue(PARAM_TRIGGER_LIMIT);
    timeWindowParam   = apvts.getRawParameterValue(PARAM_TIME_WINDOW);
    ampWindowParam    = apvts.getRawParameterValue(PARAM_AMP_WINDOW);
    syncDrawParam     = apvts.getRawParameterValue(PARAM_SYNC_DRAW);
    channelParam      = apvts.getRawParameterValue(PARAM_CHANNEL);
    freezeParam       = apvts.getRawParameterValue(PARAM_FREEZE);
    dcKillParam       = apvts.getRawParameterValue(PARAM_DC_KILL);

    // Initialize peak arrays
    CPoint tmp;
    for (int j = 0; j < OSC_WIDTH * 2; j += 2)
    {
        tmp.x = j / 2;
        tmp.y = OSC_HEIGHT * 0.5 - 1;
        peaks.push_back(tmp);
        peaks.push_back(tmp);

        tmp.x = j / 2;
        tmp.y = OSC_HEIGHT * 0.5 - 1;
        copy.push_back(tmp);
        copy.push_back(tmp);
    }

    resetState();
}

SmexoscopeProcessor::~SmexoscopeProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout SmexoscopeProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PARAM_TRIGGER_SPEED, 1), "Internal Trigger Speed",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PARAM_TRIGGER_TYPE, 1), "Trigger Type",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PARAM_TRIGGER_LEVEL, 1), "Trigger Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PARAM_TRIGGER_LIMIT, 1), "Trigger Limit",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PARAM_TIME_WINDOW, 1), "Time Window",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.75f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PARAM_AMP_WINDOW, 1), "Amp Window",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PARAM_SYNC_DRAW, 1), "Sync Redraw",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PARAM_CHANNEL, 1), "Channel",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PARAM_FREEZE, 1), "Freeze",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(PARAM_DC_KILL, 1), "DC Killer",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f), 0.0f));

    return { params.begin(), params.end() };
}

void SmexoscopeProcessor::resetState()
{
    index = 0;
    counter = 1.0;
    lastTriggerSamples = 0.0;
    maxPeak = -MAX_FLOAT;
    minPeak = MAX_FLOAT;
    previousSample = 0.f;
    triggerPhase = 0.0;
    triggerLimitPhase = 0;
    dcKill_ = dcFilterTemp_ = 0.0;
}

void SmexoscopeProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;
    resetState();
}

bool SmexoscopeProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

double SmexoscopeProcessor::timeKnobToBeats(double x)
{
    return (x + 0.005) * 4.0 + 0.5;
}

double SmexoscopeProcessor::tempoBeatsForKnob(double x)
{
    // Discrete tempo values: 0.5 (1/8), 1 (1/4), 2 (2/4), 3 (3/4), 4 (4/4)
    constexpr double steps[] = { 0.5, 1.0, 2.0, 3.0, 4.0 };
    int idx = static_cast<int>(x * 4.0 + 0.5);
    if (idx < 0) idx = 0;
    if (idx > 4) idx = 4;
    return steps[idx];
}

juce::String SmexoscopeProcessor::tempoDisplayForKnob(double x)
{
    constexpr const char* labels[] = { "1/8", "1/4", "2/4", "3/4", "4/4" };
    int idx = static_cast<int>(x * 4.0 + 0.5);
    if (idx < 0) idx = 0;
    if (idx > 4) idx = 4;
    return labels[idx];
}

void SmexoscopeProcessor::convertTimeInfo(FFXTimeInfo* ffxtime)
{
    ffxtime->isValid = false;
    ffxtime->sampleRate = currentSampleRate;
    ffxtime->tempo = 120.0;
    ffxtime->tempoBPS = 2.0;
    ffxtime->numSamplesInBeat = currentSampleRate / 2.0;
    ffxtime->samplePos = 0.0;

    if (auto* playHead = getPlayHead())
    {
        if (auto posInfo = playHead->getPosition())
        {
            ffxtime->isValid = true;

            if (auto samplePos = posInfo->getTimeInSamples())
                ffxtime->samplePos = static_cast<double>(*samplePos);

            ffxtime->tempoIsValid = posInfo->getBpm().hasValue();
            if (ffxtime->tempoIsValid)
            {
                ffxtime->tempo = *posInfo->getBpm();
                ffxtime->tempoBPS = ffxtime->tempo / 60.0;
                ffxtime->numSamplesInBeat = currentSampleRate / ffxtime->tempoBPS;
            }

            ffxtime->ppqPosIsValid = posInfo->getPpqPosition().hasValue();
            if (ffxtime->ppqPosIsValid)
                ffxtime->ppqPos = *posInfo->getPpqPosition();

            ffxtime->barsIsValid = posInfo->getPpqPositionOfLastBarStart().hasValue();
            if (ffxtime->barsIsValid)
                ffxtime->barStartPos = *posInfo->getPpqPositionOfLastBarStart();
            else
                ffxtime->barStartPos = ffxtime->ppqPos;

            ffxtime->timeSigIsValid = posInfo->getTimeSignature().hasValue();
            if (ffxtime->timeSigIsValid)
            {
                ffxtime->timeSigNumerator = posInfo->getTimeSignature()->numerator;
                ffxtime->timeSigDenominator = posInfo->getTimeSignature()->denominator;
                if (ffxtime->timeSigNumerator <= 0)
                    ffxtime->timeSigNumerator = 4;
            }

            ffxtime->samplesToNextBarIsValid = ffxtime->tempoIsValid && ffxtime->ppqPosIsValid
                                            && ffxtime->barsIsValid && ffxtime->timeSigIsValid;

            if (ffxtime->samplesToNextBarIsValid)
            {
                double distanceToNextBarPPQ;
                if (std::fabs(ffxtime->barStartPos - ffxtime->ppqPos) < 1e-12)
                    distanceToNextBarPPQ = 0.0;
                else
                    distanceToNextBarPPQ = ffxtime->barStartPos + (double)(ffxtime->timeSigNumerator) - ffxtime->ppqPos;

                while (distanceToNextBarPPQ < 0.0)
                    distanceToNextBarPPQ += (double)(ffxtime->timeSigNumerator);
                while (distanceToNextBarPPQ >= (double)(ffxtime->timeSigNumerator))
                    distanceToNextBarPPQ -= (double)(ffxtime->timeSigNumerator);

                ffxtime->numSamplesToNextBar = (distanceToNextBarPPQ * currentSampleRate * 60.0) / ffxtime->tempo;
                if (ffxtime->numSamplesToNextBar < 0)
                    ffxtime->numSamplesToNextBar = 0;
            }

            ffxtime->playbackIsOccuring = posInfo->getIsPlaying();
            // playbackChanged is not directly available in JUCE, approximate:
            ffxtime->playbackChanged = false;
        }
    }
}

void SmexoscopeProcessor::processSub(const float* samples, int sampleFrames)
{
    float freezeVal = freezeParam->load();
    if (freezeVal > 0.5f)
    {
        resetState();
        return;
    }

    float ampWindowVal    = ampWindowParam->load();
    float triggerLevelVal = triggerLevelParam->load();
    float triggerTypeVal  = triggerTypeParam->load();
    float triggerLimitVal = triggerLimitParam->load();
    float triggerSpeedVal = triggerSpeedParam->load();
    float timeWindowVal   = timeWindowParam->load();
    float dcKillVal       = dcKillParam->load();

    float gain = std::pow(10.f, ampWindowVal * 6.f - 3.f);
    float trigLevel = (triggerLevelVal * 2.f - 1.f);
    long trigType = (long)(triggerTypeVal * kNumTriggerTypes + 0.0001);
    long trigLimit = (long)(std::pow(10.f, triggerLimitVal * 4.f));
    double trigSpeed = std::pow(10.0, 2.5 * triggerSpeedVal - 5.0);
    double counterSpeed = std::pow(10.f, -timeWindowVal * 5.f + 1.5);
    double R = 1.0 - 250.0 / currentSampleRate;
    bool dcOn = dcKillVal > 0.5f;

    FFXTimeInfo info;
    convertTimeInfo(&info);

    double beatSamples = info.sampleRate / info.tempoBPS;
    double loopBeats = timeKnobToBeats(timeWindowVal);
    if (loopBeats < 1.0) loopBeats = 1.0;
    double loopLength = std::floor(loopBeats) * beatSamples;
    if (trigType == kTriggerTempo)
    {
        double tempoBeats = tempoBeatsForKnob(timeWindowVal);
        double tempoLoopLength = tempoBeats * beatSamples;
        counterSpeed = OSC_WIDTH / tempoLoopLength;
        loopLength = tempoLoopLength;
    }

    for (int i = 0; i < sampleFrames; i++)
    {
        double t = info.samplePos + i;

        // DC filter
        dcKill_ = samples[i] - dcFilterTemp_ + R * dcKill_;
        dcFilterTemp_ = samples[i];
        if (std::fabs(dcKill_) < 1e-10)
            dcKill_ = 0.0;

        float sample = dcOn ? (float)dcKill_ : samples[i];
        sample = clip(sample * gain, 1.f);

        bool trigger = false;

        switch (trigType)
        {
        case kTriggerInternal:
            triggerPhase += trigSpeed;
            if (triggerPhase >= 1.0)
            {
                triggerPhase -= 1.0;
                trigger = true;
            }
            break;
        case kTriggerTempo:
        {
            double diff = t - lastTriggerSamples;
            if (diff > loopLength || diff < 0)
                trigger = true;
            if (i == 0 && info.playbackChanged && info.playbackIsOccuring)
                trigger = true;
            break;
        }
        case kTriggerRising:
            if (sample >= trigLevel && previousSample < trigLevel)
                trigger = true;
            break;
        case kTriggerFalling:
            if (sample <= trigLevel && previousSample > trigLevel)
                trigger = true;
            break;
        case kTriggerFree:
            if (index >= OSC_WIDTH)
                trigger = true;
            break;
        }

        triggerLimitPhase++;
        if (trigger && triggerLimitPhase < trigLimit
            && trigType != kTriggerFree && trigType != kTriggerInternal && trigType != kTriggerTempo)
            trigger = false;

        if (trigger)
        {
            for (unsigned long j = index * 2; j < OSC_WIDTH * 2; j += 2)
                peaks[j].y = peaks[j + 1].y = OSC_HEIGHT * 0.5 - 1;

            for (unsigned long j = 0; j < OSC_WIDTH * 2; j++)
                copy[j].y = peaks[j].y;

            index = 0;
            counter = 1.0;
            maxPeak = -MAX_FLOAT;
            minPeak = MAX_FLOAT;
            triggerLimitPhase = 0;
            lastTriggerSamples = t;
        }

        if (sample > maxPeak)
        {
            maxPeak = sample;
            lastIsMax = true;
        }
        if (sample < minPeak)
        {
            minPeak = sample;
            lastIsMax = false;
        }

        counter += counterSpeed;

        if (counter >= 1.0)
        {
            if (index < OSC_WIDTH)
            {
                double max_Y = (OSC_HEIGHT * 0.5 - maxPeak * 0.5 * OSC_HEIGHT) - 1.0;
                double min_Y = (OSC_HEIGHT * 0.5 - minPeak * 0.5 * OSC_HEIGHT) - 1.0;

                peaks[(index << 1)].y     = lastIsMax ? min_Y : max_Y;
                peaks[(index << 1) + 1].y = lastIsMax ? max_Y : min_Y;

                index++;
            }

            maxPeak = -MAX_FLOAT;
            minPeak = MAX_FLOAT;
            counter -= 1.0;
        }

        previousSample = sample;
    }
}

void SmexoscopeProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Pass-through: input -> output (already in-place for JUCE)

    // Pick channel for oscilloscope
    float channelVal = channelParam->load();
    int ch = (channelVal > 0.5f) ? 1 : 0;
    if (ch >= buffer.getNumChannels()) ch = 0;

    processSub(buffer.getReadPointer(ch), buffer.getNumSamples());
}

juce::String SmexoscopeProcessor::getDisplayText(const juce::String& paramId) const
{
    float trigTypeVal = triggerTypeParam->load();
    long trigType = (long)(trigTypeVal * kNumTriggerTypes + 0.0001);

    if (paramId == PARAM_TIME_WINDOW)
    {
        float val = timeWindowParam->load();
        if (trigType == kTriggerTempo)
        {
            return tempoDisplayForKnob(val);
        }
        else
        {
            double counterSpeed = std::pow(10.0, -val * 5.0 + 1.5);
            return juce::String(counterSpeed, 2);
        }
    }
    else if (paramId == PARAM_AMP_WINDOW)
    {
        float val = ampWindowParam->load();
        float gain = std::pow(10.f, val * 6.f - 3.f);
        return juce::String(gain, 2);
    }
    else if (paramId == PARAM_TRIGGER_SPEED)
    {
        float val = triggerSpeedParam->load();
        double speed = std::pow(10.0, 2.5 * val - 5.0) * currentSampleRate;
        return juce::String(speed, 2);
    }
    else if (paramId == PARAM_TRIGGER_LIMIT)
    {
        float val = triggerLimitParam->load();
        long limit = (long)(std::pow(10.f, val * 4.f));
        return juce::String(limit);
    }

    return {};
}

void SmexoscopeProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SmexoscopeProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* SmexoscopeProcessor::createEditor()
{
    return new SmexoscopeEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SmexoscopeProcessor();
}
