#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <random>

// Parameter IDs - global parameters
const juce::String feedbackID = "feedback";
const juce::String delayTimeID = "delayTime";
const juce::String masterVolumeID = "masterVolume";

// Helper function to create voice parameter IDs
static juce::String createVoiceParamID(int voiceIndex, int paramType)
{
    juce::String baseNames[NUM_PARAMS_PER_VOICE] = {
        "freq", "duration", "amp", "balance", "noise", "distortion"
    };
    
    return baseNames[paramType] + juce::String(voiceIndex);
}

// Constructor
OnePingOnlyProcessor::OnePingOnlyProcessor()
    : AudioProcessor(BusesProperties()
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "OnePingOnly",
                {
                    // Global parameters
                    std::make_unique<juce::AudioParameterFloat>(feedbackID, "Feedback",
                                                               juce::NormalisableRange<float>(0.0f, 0.95f), 0.6f),
                    std::make_unique<juce::AudioParameterFloat>(delayTimeID, "Delay Time",
                                                               juce::NormalisableRange<float>(0.0f, 1.0f), 0.25f),
                    std::make_unique<juce::AudioParameterFloat>(masterVolumeID, "Master Volume",
                                                               juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f)
                }),
      currentSampleRate(44100.0),
      currentProgram(0)
{
    // Initialize frequency table for MIDI notes
    for (int i = 0; i < 128; ++i)
    {
        // Convert MIDI note to frequency (Hz)
        midiNoteFrequencies[static_cast<size_t>(i)] = 440.0 * std::pow(2.0, (i - 69) / 12.0);
    }
    
    // Create voice parameters
    for (int voiceIndex = 0; voiceIndex < NUM_VOICES; ++voiceIndex)
    {
        for (int paramType = 0; paramType < NUM_PARAMS_PER_VOICE; ++paramType)
        {
            juce::String paramID = createVoiceParamID(voiceIndex, paramType);
            juce::String paramName = "Voice " + juce::String(voiceIndex) + " ";
            
            // Default values and ranges depend on parameter type
            float defaultValue = 0.0f;
            juce::NormalisableRange<float> range(0.0f, 1.0f);
            
            switch (paramType)
            {
                case kFreq:
                    paramName += "Frequency";
                    defaultValue = static_cast<float>(voiceIndex) / NUM_VOICES;
                    break;
                    
                case kDuration:
                    paramName += "Duration";
                    defaultValue = juce::Random::getSystemRandom().nextFloat() * 0.25f;
                    break;
                    
                case kAmp:
                    paramName += "Amplitude";
                    defaultValue = 1.0f;
                    break;
                    
                case kBalance:
                    paramName += "Balance";
                    defaultValue = juce::Random::getSystemRandom().nextFloat();
                    break;
                    
                case kNoise:
                    paramName += "Noise";
                    defaultValue = 0.0f;
                    break;
                    
                case kDistortion:
                    paramName += "Distortion";
                    defaultValue = juce::Random::getSystemRandom().nextFloat() * 0.5f;
                    break;
            }
            
            // Add parameter to the APVTS
            parameters.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>(
                paramID, paramName, range, defaultValue));
        }
    }
    
    // Program names
    for (int i = 0; i < 12; ++i)
    {
        if (i == 0)
            programNames[static_cast<size_t>(i)] = "Init";
        else
            programNames[static_cast<size_t>(i)] = "Program " + juce::String(i);
    }
    
    // Initialize parameter state after adding them all
    parameters.state = juce::ValueTree("OnePingOnlyParams");
    
    // Add listener for parameter changes
    parameters.state.addListener(this);
    
    // Initialize voices
    for (auto& voice : voices)
    {
        voice.active = false;
        voice.note = -1;
    }
}

OnePingOnlyProcessor::~OnePingOnlyProcessor()
{
    parameters.state.removeListener(this);
}

void OnePingOnlyProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;
    
    // Create delays with maximum delay time of 3 seconds
    delayLeft = std::make_unique<Delay>(static_cast<int>(sampleRate * 3.0));
    delayRight = std::make_unique<Delay>(static_cast<int>(sampleRate * 3.0));
    
    // Update parameters for new sample rate
    updateAllVoicesFromParameters();
    
    // Set delay from parameters
    delayTime = *parameters.getRawParameterValue(delayTimeID);
    feedback = *parameters.getRawParameterValue(feedbackID);
    masterVolume = *parameters.getRawParameterValue(masterVolumeID);
    
    int delaySamples = static_cast<int>(delayTime * currentSampleRate);
    delayLeft->setDelay(delaySamples);
    delayRight->setDelay(delaySamples);
    
    delayLeft->setFeedback(feedback);
    delayRight->setFeedback(feedback);
}

void OnePingOnlyProcessor::releaseResources()
{
    // Release any resources when plugin is not processing
}

bool OnePingOnlyProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Only support stereo output
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    // Input should be empty for a synth
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::disabled())
        return false;
    
    return true;
}

void OnePingOnlyProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    // Clear output buffer
    buffer.clear();
    
    // Process MIDI events
    for (const auto metadata : midiMessages)
    {
        handleMidiEvent(metadata.getMessage());
    }
    
    const int numSamples = buffer.getNumSamples();
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);
    
    // Process each sample
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Sum all active voices
        float leftOutput = 0.0f;
        float rightOutput = 0.0f;
        
        // Process each voice
        for (int voiceIndex = 0; voiceIndex < NUM_VOICES; ++voiceIndex)
        {
            if (voices[static_cast<size_t>(voiceIndex)].active)
            {
                float leftVoice = 0.0f;
                float rightVoice = 0.0f;
                
                processVoice(voiceIndex, leftVoice, rightVoice);
                
                leftOutput += leftVoice;
                rightOutput += rightVoice;
            }
        }
        
        // Process delay
        leftOutput = delayLeft->process(leftOutput);
        rightOutput = delayRight->process(rightOutput);
        
        // Apply master volume
        leftChannel[sample] = leftOutput * masterVolume;
        rightChannel[sample] = rightOutput * masterVolume;
    }
}

void OnePingOnlyProcessor::handleMidiEvent(const juce::MidiMessage& midiMessage)
{
    if (midiMessage.isNoteOn())
    {
        noteOn(midiMessage.getNoteNumber(), midiMessage.getFloatVelocity());
    }
    else if (midiMessage.isNoteOff())
    {
        noteOff(midiMessage.getNoteNumber(), midiMessage.getFloatVelocity());
    }
    else if (midiMessage.isAllNotesOff())
    {
        for (auto& voice : voices)
            voice.active = false;
    }
}

void OnePingOnlyProcessor::noteOn(int midiNoteNumber, float velocity)
{
    if (midiNoteNumber < 0 || midiNoteNumber >= NUM_VOICES)
        return;
    
    int voiceIndex = midiNoteNumber;
    
    // Activate the voice
    voices[static_cast<size_t>(voiceIndex)].active = true;
    voices[static_cast<size_t>(voiceIndex)].note = midiNoteNumber;
    voices[static_cast<size_t>(voiceIndex)].amplitude = velocity;
    
    // Update parameters for the voice
    updateVoiceFromParameters(voiceIndex);
}

void OnePingOnlyProcessor::noteOff(int midiNoteNumber, float /*velocity*/)
{
    if (midiNoteNumber < 0 || midiNoteNumber >= NUM_VOICES)
        return;
    
    // Deactivate the voice
    int voiceIndex = midiNoteNumber;
    voices[static_cast<size_t>(voiceIndex)].active = false;
}

void OnePingOnlyProcessor::updateVoiceFromParameters(int voiceIndex)
{
    // Update voice parameters from APVTS
    if (voiceIndex < 0 || voiceIndex >= NUM_VOICES)
        return;
    
    // Get voice parameters
    juce::String freqID = createVoiceParamID(voiceIndex, kFreq);
    juce::String durationID = createVoiceParamID(voiceIndex, kDuration);
    juce::String ampID = createVoiceParamID(voiceIndex, kAmp);
    juce::String balanceID = createVoiceParamID(voiceIndex, kBalance);
    juce::String noiseID = createVoiceParamID(voiceIndex, kNoise);
    juce::String distortionID = createVoiceParamID(voiceIndex, kDistortion);
    
    // These parameters could be used in future enhancements
    // float freq = *parameters.getRawParameterValue(freqID);
    // float duration = *parameters.getRawParameterValue(durationID);
    // float amp = *parameters.getRawParameterValue(ampID);
    
    float balance = *parameters.getRawParameterValue(balanceID);
    float noise = *parameters.getRawParameterValue(noiseID);
    float distortion = *parameters.getRawParameterValue(distortionID);
    
    // Configure the voice filter
    double freq_hz = midiNoteFrequencies[static_cast<size_t>(voiceIndex)];
    double omega = 2.0 * juce::MathConstants<double>::pi * freq_hz / currentSampleRate;
    // double sin_omega = std::sin(omega); // Not used in current implementation
    double cos_omega = std::cos(omega);
    
    size_t voiceIdx = static_cast<size_t>(voiceIndex);
    
    voices[voiceIdx].alpha = 2.0f * static_cast<float>(cos_omega);
    voices[voiceIdx].beta = 0.99f;
    voices[voiceIdx].gamma = -0.99f;
    
    // Set the other voice parameters
    voices[voiceIdx].balance = balance;
    voices[voiceIdx].noiseAmount = noise;
    voices[voiceIdx].distortionAmount = distortion;
    
    // Reset filter state
    voices[voiceIdx].in = 0.0f;
    voices[voiceIdx].in_1 = 0.0f;
    voices[voiceIdx].in_2 = 0.0f;
    voices[voiceIdx].out_1 = 0.0f;
    voices[voiceIdx].out_2 = 0.0f;
    
    // Reset envelope state
    voices[voiceIdx].d1 = 0.0f;
    voices[voiceIdx].d2 = 0.0f;
}

void OnePingOnlyProcessor::updateAllVoicesFromParameters()
{
    for (int i = 0; i < NUM_VOICES; ++i)
        updateVoiceFromParameters(i);
    
    // Update global parameters
    delayTime = *parameters.getRawParameterValue(delayTimeID);
    feedback = *parameters.getRawParameterValue(feedbackID);
    masterVolume = *parameters.getRawParameterValue(masterVolumeID);
    
    if (delayLeft && delayRight)
    {
        int delaySamples = static_cast<int>(delayTime * currentSampleRate);
        delayLeft->setDelay(delaySamples);
        delayRight->setDelay(delaySamples);
        delayLeft->setFeedback(feedback);
        delayRight->setFeedback(feedback);
    }
}

void OnePingOnlyProcessor::processVoice(int voiceIndex, float& leftOut, float& rightOut)
{
    auto& voice = voices[static_cast<size_t>(voiceIndex)];
    
    // Noise input (1.0 gives a ping, any less adds noise)
    float input = 1.0f;
    if (voice.noiseAmount > 0.0f)
    {
        input = voice.noiseAmount * (juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f) + (1.0f - voice.noiseAmount);
    }
    
    // Decay envelope
    voice.d2 = voice.d1;
    voice.d1 = voice.amplitude * 0.001f + voice.d1 * 0.999f;
    float envelope = voice.d1 - voice.d2;
    
    // Make sure the envelope doesn't go negative
    if (envelope < 0.0f)
        envelope = 0.0f;
    
    // Apply the envelope to the input
    voice.in = input * envelope;
    
    // Resonant filter (simulates the ping sound)
    float out = voice.alpha * voice.out_1 + voice.beta * voice.out_2 + voice.in - voice.in_1 * voice.alpha + voice.in_2 * voice.gamma;
    
    // Distortion effect
    if (voice.distortionAmount > 0.0f)
    {
        out = std::tanh(out * (1.0f + voice.distortionAmount * 10.0f)) / (1.0f + voice.distortionAmount * 10.0f);
    }
    
    // Update filter state
    voice.in_2 = voice.in_1;
    voice.in_1 = voice.in;
    voice.out_2 = voice.out_1;
    voice.out_1 = out;
    
    // Apply balance/panning
    float pan = voice.balance * 2.0f - 1.0f; // Convert 0-1 to -1 to +1
    leftOut = out * (1.0f - std::max(0.0f, pan));
    rightOut = out * (1.0f + std::min(0.0f, pan));
}

juce::AudioProcessorEditor* OnePingOnlyProcessor::createEditor()
{
    return new OnePingOnlyEditor (*this);
}

bool OnePingOnlyProcessor::hasEditor() const
{
    return true;
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
    // Allow 3 seconds of delay plus some decay time
    return 5.0;
}

int OnePingOnlyProcessor::getNumPrograms()
{
    return 12;
}

int OnePingOnlyProcessor::getCurrentProgram()
{
    return currentProgram;
}

void OnePingOnlyProcessor::setCurrentProgram(int index)
{
    if (index < 0 || index >= getNumPrograms())
        return;
    
    currentProgram = index;
    
    // Initialize random values for program (except for program 0 which is "Init")
    std::mt19937 rng(static_cast<unsigned int>(index));  // Seed with program index for reproducibility
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    for (int voiceIndex = 0; voiceIndex < NUM_VOICES; ++voiceIndex)
    {
        // Set different random values for each parameter
        float freqRandom = (index == 0) ? static_cast<float>(voiceIndex) / NUM_VOICES : dist(rng);
        float durationRandom = (index == 0) ? dist(rng) * 0.25f : dist(rng) * 0.5f;
        float ampRandom = (index == 0) ? 1.0f : 0.5f + dist(rng) * 0.5f;
        float balanceRandom = (index == 0) ? dist(rng) : dist(rng);
        float noiseRandom = (index == 0) ? 0.0f : dist(rng) * 0.3f;
        float distortionRandom = (index == 0) ? dist(rng) * 0.5f : dist(rng) * 0.7f;
        
        // Update parameters
        parameters.getParameter(createVoiceParamID(voiceIndex, kFreq))->setValueNotifyingHost(freqRandom);
        parameters.getParameter(createVoiceParamID(voiceIndex, kDuration))->setValueNotifyingHost(durationRandom);
        parameters.getParameter(createVoiceParamID(voiceIndex, kAmp))->setValueNotifyingHost(ampRandom);
        parameters.getParameter(createVoiceParamID(voiceIndex, kBalance))->setValueNotifyingHost(balanceRandom);
        parameters.getParameter(createVoiceParamID(voiceIndex, kNoise))->setValueNotifyingHost(noiseRandom);
        parameters.getParameter(createVoiceParamID(voiceIndex, kDistortion))->setValueNotifyingHost(distortionRandom);
    }
    
    // Set global parameters based on program
    float delayRandom = (index == 0) ? 0.25f : 0.1f + dist(rng) * 0.4f;
    float feedbackRandom = (index == 0) ? 0.6f : 0.4f + dist(rng) * 0.5f;
    float masterRandom = (index == 0) ? 1.0f : 0.7f + dist(rng) * 0.3f;
    
    parameters.getParameter(delayTimeID)->setValueNotifyingHost(delayRandom);
    parameters.getParameter(feedbackID)->setValueNotifyingHost(feedbackRandom);
    parameters.getParameter(masterVolumeID)->setValueNotifyingHost(masterRandom);
    
    updateAllVoicesFromParameters();
}

const juce::String OnePingOnlyProcessor::getProgramName(int index)
{
    if (index < 0 || index >= getNumPrograms())
        return {};
    
    return programNames[static_cast<size_t>(index)];
}

void OnePingOnlyProcessor::changeProgramName(int index, const juce::String& newName)
{
    if (index < 0 || index >= getNumPrograms())
        return;
    
    programNames[static_cast<size_t>(index)] = newName;
}

void OnePingOnlyProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    
    // Add the current program index to the state
    state.setProperty("currentProgram", currentProgram, nullptr);
    
    // Add custom program names
    for (int i = 0; i < getNumPrograms(); ++i)
    {
        state.setProperty("programName" + juce::String(i), programNames[static_cast<size_t>(i)], nullptr);
    }
    
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void OnePingOnlyProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
            
            // Restore current program
            if (xmlState->hasAttribute("currentProgram"))
            {
                currentProgram = xmlState->getIntAttribute("currentProgram", 0);
            }
            
            // Restore program names
            for (int i = 0; i < getNumPrograms(); ++i)
            {
                juce::String nameKey = "programName" + juce::String(i);
                if (xmlState->hasAttribute(nameKey))
                {
                    programNames[static_cast<size_t>(i)] = xmlState->getStringAttribute(nameKey, "Program " + juce::String(i));
                }
            }
            
            updateAllVoicesFromParameters();
        }
    }
}

void OnePingOnlyProcessor::valueTreePropertyChanged(juce::ValueTree& /*tree*/, const juce::Identifier& /*property*/)
{
    // React to parameter changes
    updateAllVoicesFromParameters();
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OnePingOnlyProcessor();
}