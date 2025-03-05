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
      // Initialize class members in the order they're declared in the header file to avoid warnings
      // Fields need to be initialized in the same order they're declared in the class definition
      delayTime(0.25f),
      feedback(0.6f),
      masterVolume(1.0f),
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
    
    // IMPORTANT: Mark this plugin as a synthesizer to ensure it receives MIDI
    setPlayConfigDetails(0, 2, getSampleRate(), getBlockSize());
    
    // Already configured to accept MIDI in the AudioProcessor initialization
    
    // Initialize voices
    for (auto& voice : voices)
    {
        voice.active = false;
        voice.note = -1;
        
        // Initialize gain values to ensure some sound even for default values
        voice.gain = 0.4f;
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
    
    // Also let's directly handle any MIDI messages first, as a backup
    // This ensures that notes are triggered even if our MIDI buffering doesn't work
    for (const auto metadata : midiMessages)
    {
        handleMidiEvent(metadata.getMessage());
    }
    
    // Extract MIDI events and store them like in the original
    midiNotes.clear();
    int midiNoteCount = 0;
    
    for (const auto metadata : midiMessages)
    {
        const auto& msg = metadata.getMessage();
        
        if (msg.isNoteOn())
        {
            int note = msg.getNoteNumber();
            float velocity = msg.getFloatVelocity();
            int deltaFrames = static_cast<int>(metadata.samplePosition);
            
            MidiNoteData noteData;
            noteData.note = note;
            noteData.velocity = velocity;
            noteData.deltaFrames = deltaFrames;
            
            // Add to our notes list
            if (midiNoteCount < MAX_MIDI_NOTES)
            {
                midiNotes.push_back(noteData);
                midiNoteCount++;
            }
        }
    }
    
    const int numSamples = buffer.getNumSamples();
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);
    
    // -- EXACTLY MIRRORING THE ORIGINAL processReplacing FUNCTION --
    
    // Current note we're processing
    int noteIndex = 0;
    
    // Current sample position
    int samplePosition = 0;
    
    // Variables for processing
    float out, o, oL, oR, noise;
    bool moreNotes = (midiNoteCount > 0);
    bool moreThanOne = false;
    int voiceIndex;
    
    // If there are no MIDI notes to process, go straight to the no-notes section
    if (!moreNotes)
        goto NoMoreNotes;
    
    // Process samples with MIDI notes
    while (samplePosition < numSamples)
    {
        // Generate noise (same as original)
        noise = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
        
        // Check if we hit a MIDI note on this sample
        if (samplePosition == midiNotes[static_cast<size_t>(noteIndex)].deltaFrames)
        {
            // Get the note and its voice
            voiceIndex = midiNotes[static_cast<size_t>(noteIndex)].note;
            
            if (voiceIndex >= 0 && voiceIndex < NUM_VOICES)
            {
                auto& voice = voices[static_cast<size_t>(voiceIndex)];
                
                // This is crucial - directly mirroring the original implementation
                // g[index] * pNotes++->Velocity
                voice.in = voice.gain * midiNotes[static_cast<size_t>(noteIndex)].velocity;
                voice.active = true;
                
                // Debug voice activation
                DBG("Voice " + juce::String(voiceIndex) + " activated at sample " + juce::String(samplePosition) + 
                    " with gain " + juce::String(voice.gain) + " and velocity " + juce::String(midiNotes[static_cast<size_t>(noteIndex)].velocity));
            }
            
            noteIndex++;
            
            // Check for multiple notes at the same time
            while (noteIndex < midiNoteCount && 
                   midiNotes[static_cast<size_t>(noteIndex)].deltaFrames == samplePosition)
            {
                voiceIndex = midiNotes[static_cast<size_t>(noteIndex)].note;
                
                if (voiceIndex >= 0 && voiceIndex < NUM_VOICES)
                {
                    auto& voice = voices[static_cast<size_t>(voiceIndex)];
                    voice.in = voice.gain * midiNotes[static_cast<size_t>(noteIndex)].velocity;
                    voice.active = true;
                    
                    // Debug multiple voices
                    DBG("Additional voice " + juce::String(voiceIndex) + " activated in same sample");
                }
                
                noteIndex++;
                moreThanOne = true;
            }
            
            // Initialize outputs
            oL = oR = o = 0.0f;
            
            // Process all voices - exactly as in original
            for (int i = 0; i < NUM_VOICES; i++)
            {
                auto& voice = voices[static_cast<size_t>(i)];
                
                out = voice.beta * (voice.in - voice.out_2) + 
                      voice.alpha * (voice.in_1 - voice.out_1) + voice.in_2;
                
                voice.out_2 = voice.out_1;
                voice.out_1 = out;
                voice.in_2 = voice.in_1;
                voice.in_1 = voice.in;
                
                // Always get the latest parameter values for active voices
                // Get current parameters for this voice to enable live tweaking
                if (voice.active)
                {
                    // Re-read voice parameters from the APVTS
                    juce::String freqID = createVoiceParamID(i, kFreq);
                    juce::String durationID = createVoiceParamID(i, kDuration);
                    juce::String ampID = createVoiceParamID(i, kAmp);
                    juce::String balanceID = createVoiceParamID(i, kBalance);
                    juce::String noiseID = createVoiceParamID(i, kNoise);
                    juce::String distortionID = createVoiceParamID(i, kDistortion);
                    
                    // Update key parameters that affect the sound in real-time
                    voice.noiseAmount = *parameters.getRawParameterValue(noiseID);
                    voice.balance = *parameters.getRawParameterValue(balanceID);
                    
                    // Get distortion parameter
                    float distVal = *parameters.getRawParameterValue(distortionID) * 10.0f;
                    voice.d1 = distVal;
                    if (distVal > 1.0f)
                        voice.d2 = (1.0f + distVal) * (1.0f - 0.8f * (distVal - 1.0f) / 19.0f);
                    else
                        voice.d2 = 1.0f + distVal;
                }
                
                // The same noise and distortion calculation from original
                o = (out - voice.in) * (1.0f + voice.noiseAmount * noise);
                o *= voice.d2 / (1.0f + voice.d1 * o * o);
                
                oL += voice.balance * o;
                oR += (1.0f - voice.balance) * o;
            }
            
            // Process through delay and write to output
            leftChannel[samplePosition] = delayLeft->getVal(oL) * masterVolume;
            rightChannel[samplePosition] = delayRight->getVal(oR) * masterVolume;
            
            // Reset input values
            if (moreThanOne)
            {
                for (int i = 0; i < NUM_VOICES; i++)
                {
                    voices[static_cast<size_t>(i)].in = 0.0f;
                }
                moreThanOne = false;
            }
            else if (voiceIndex >= 0 && voiceIndex < NUM_VOICES)
            {
                voices[static_cast<size_t>(voiceIndex)].in = 0.0f;
            }
            
            samplePosition++;
            
            // Check if we're out of notes
            if (noteIndex >= midiNoteCount)
                goto NoMoreNotes;
        }
        else
        {
            // No new notes at this sample, process normally
            oL = oR = o = 0.0f;
            
            for (int i = 0; i < NUM_VOICES; i++)
            {
                auto& voice = voices[static_cast<size_t>(i)];
                
                // This is from the original when there are no new MIDI inputs
                out = -voice.beta * voice.out_2 + 
                      voice.alpha * (voice.in_1 - voice.out_1) + voice.in_2;
                
                voice.out_2 = voice.out_1;
                voice.out_1 = out;
                voice.in_2 = voice.in_1;
                voice.in_1 = 0.0f;
                
                // Always get the latest parameter values for active voices
                if (voice.active)
                {
                    // Re-read voice parameters from the APVTS
                    juce::String freqID = createVoiceParamID(i, kFreq);
                    juce::String durationID = createVoiceParamID(i, kDuration);
                    juce::String ampID = createVoiceParamID(i, kAmp);
                    juce::String balanceID = createVoiceParamID(i, kBalance);
                    juce::String noiseID = createVoiceParamID(i, kNoise);
                    juce::String distortionID = createVoiceParamID(i, kDistortion);
                    
                    // Update key parameters that affect the sound in real-time
                    voice.noiseAmount = *parameters.getRawParameterValue(noiseID);
                    voice.balance = *parameters.getRawParameterValue(balanceID);
                    
                    // Get distortion parameter
                    float distVal = *parameters.getRawParameterValue(distortionID) * 10.0f;
                    voice.d1 = distVal;
                    if (distVal > 1.0f)
                        voice.d2 = (1.0f + distVal) * (1.0f - 0.8f * (distVal - 1.0f) / 19.0f);
                    else
                        voice.d2 = 1.0f + distVal;
                }
                
                o = out * (1.0f + voice.noiseAmount * noise);
                o *= voice.d2 / (1.0f + voice.d1 * o * o);
                
                oL += voice.balance * o;
                oR += (1.0f - voice.balance) * o;
            }
            
            leftChannel[samplePosition] = delayLeft->getVal(oL) * masterVolume;
            rightChannel[samplePosition] = delayRight->getVal(oR) * masterVolume;
            
            samplePosition++;
        }
    }
    
    goto end;
    
    // -- Handle the case when there are no more MIDI notes to process --
NoMoreNotes:
    {
        int remainingSamples = numSamples - samplePosition;
        
        if (remainingSamples > 2)
        {
            // First sample processing
            noise = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
            oL = oR = o = 0.0f;
            
            for (int i = 0; i < NUM_VOICES; i++)
            {
                auto& voice = voices[static_cast<size_t>(i)];
                
                out = -voice.beta * voice.out_2 + 
                      voice.alpha * (voice.in_1 - voice.out_1) + voice.in_2;
                
                voice.out_2 = voice.out_1;
                voice.out_1 = out;
                voice.in_2 = voice.in_1;
                voice.in_1 = 0.0f;
                
                o = out * (1.0f + voice.noiseAmount * noise);
                o *= voice.d2 / (1.0f + voice.d1 * o * o);
                
                oL += voice.balance * o;
                oR += (1.0f - voice.balance) * o;
            }
            
            leftChannel[samplePosition] = delayLeft->getVal(oL) * masterVolume;
            rightChannel[samplePosition] = delayRight->getVal(oR) * masterVolume;
            samplePosition++;
            
            // Second sample processing
            noise = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
            oL = oR = o = 0.0f;
            
            for (int i = 0; i < NUM_VOICES; i++)
            {
                auto& voice = voices[static_cast<size_t>(i)];
                
                out = -voice.beta * voice.out_2 - voice.alpha * voice.out_1 + voice.in_2;
                
                voice.out_2 = voice.out_1;
                voice.out_1 = out;
                voice.in_2 = 0.0f;
                
                o = out * (1.0f + voice.noiseAmount * noise);
                o *= voice.d2 / (1.0f + voice.d1 * o * o);
                
                oL += voice.balance * o;
                oR += (1.0f - voice.balance) * o;
            }
            
            leftChannel[samplePosition] = delayLeft->getVal(oL) * masterVolume;
            rightChannel[samplePosition] = delayRight->getVal(oR) * masterVolume;
            samplePosition++;
            
            // Remaining samples - in=in_1=in_2=0
            remainingSamples = numSamples - samplePosition;
            
            for (int s = 0; s < remainingSamples; s++)
            {
                noise = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
                oL = oR = o = 0.0f;
                
                for (int i = 0; i < NUM_VOICES; i++)
                {
                    auto& voice = voices[static_cast<size_t>(i)];
                    
                    out = -voice.beta * voice.out_2 - voice.alpha * voice.out_1;
                    
                    voice.out_2 = voice.out_1;
                    voice.out_1 = out;
                    
                    o = out * (1.0f + voice.noiseAmount * noise);
                    o *= voice.d2 / (1.0f + voice.d1 * o * o);
                    
                    oL += voice.balance * o;
                    oR += (1.0f - voice.balance) * o;
                }
                
                leftChannel[samplePosition] = delayLeft->getVal(oL) * masterVolume;
                rightChannel[samplePosition] = delayRight->getVal(oR) * masterVolume;
                samplePosition++;
            }
        }
        else
        {
            // Just the remaining 1-2 samples
            for (int s = 0; s < remainingSamples; s++)
            {
                noise = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
                oL = oR = o = 0.0f;
                
                for (int i = 0; i < NUM_VOICES; i++)
                {
                    auto& voice = voices[static_cast<size_t>(i)];
                    
                    out = -voice.beta * voice.out_2 + 
                          voice.alpha * (voice.in_1 - voice.out_1) + voice.in_2;
                    
                    voice.out_2 = voice.out_1;
                    voice.out_1 = out;
                    voice.in_2 = voice.in_1;
                    voice.in_1 = 0.0f;
                    
                    o = out * (1.0f + voice.noiseAmount * noise);
                    o *= voice.d2 / (1.0f + voice.d1 * o * o);
                    
                    oL += voice.balance * o;
                    oR += (1.0f - voice.balance) * o;
                }
                
                leftChannel[samplePosition] = delayLeft->getVal(oL) * masterVolume;
                rightChannel[samplePosition] = delayRight->getVal(oR) * masterVolume;
                samplePosition++;
            }
        }
    }
end:
    
    // Check for denormals just like the original
    for (int i = 0; i < NUM_VOICES; i++)
    {
        auto& voice = voices[static_cast<size_t>(i)];
        
        if (std::fpclassify(voice.out_1) == FP_SUBNORMAL)
            voice.out_1 = 0.0f;
            
        if (std::fpclassify(voice.out_2) == FP_SUBNORMAL)
            voice.out_2 = 0.0f;
    }
}

void OnePingOnlyProcessor::handleMidiEvent(const juce::MidiMessage& midiMessage)
{
    if (midiMessage.isNoteOn())
    {
        int note = midiMessage.getNoteNumber();
        float velocity = midiMessage.getFloatVelocity();
        
        // Direct handling of MIDI notes - make sure valid notes have sound
        if (note >= 0 && note < NUM_VOICES)
        {
            // Call noteOn to set up the voice properly
            noteOn(note, velocity);
        }
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
    
    // In OnePingOnly, voices are directly mapped to MIDI note numbers - no allocation needed
    int voiceIndex = midiNoteNumber;
    auto& voice = voices[static_cast<size_t>(voiceIndex)];
    
    // Activate the voice
    voice.active = true;
    voice.note = midiNoteNumber;
    
    // Update parameters for the voice
    updateVoiceFromParameters(voiceIndex);
    
    // CRITICAL: Set the input value based on velocity and gain
    // This is what actually creates the "ping" sound in the original implementation
    voice.in = voice.gain * velocity;
    
    // Make sure voice is updated in processBlock
    DBG("NoteOn: Voice " + juce::String(voiceIndex) + " set to ACTIVE state");
    
    // Debug output to check voice triggering
    DBG("Note On: " + juce::String(midiNoteNumber) + " with velocity " + juce::String(velocity) + " and gain " + juce::String(voice.gain));
}

void OnePingOnlyProcessor::noteOff(int midiNoteNumber, float /*velocity*/)
{
    if (midiNoteNumber < 0 || midiNoteNumber >= NUM_VOICES)
        return;
    
    // DON'T deactivate the voice - the Ping effect needs to continue
    // This is a critical change - in a typical ping synth, we want the note
    // to continue resonating and allow parameter adjustments
    
    // int voiceIndex = midiNoteNumber;
    // voices[static_cast<size_t>(voiceIndex)].active = false;
    
    // Just log for debugging
    DBG("Note Off received for note " + juce::String(midiNoteNumber) + " - keeping voice active");
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
    
    // Load all the parameters
    float freq = *parameters.getRawParameterValue(freqID);
    float duration = *parameters.getRawParameterValue(durationID);
    float amp = *parameters.getRawParameterValue(ampID);
    float balance = *parameters.getRawParameterValue(balanceID);
    float noise = *parameters.getRawParameterValue(noiseID);
    float distortion = *parameters.getRawParameterValue(distortionID);
    
    size_t voiceIdx = static_cast<size_t>(voiceIndex);
    
    // SetFreq - exactly as in original implementation
    if (freq < 0.001f)
        freq = 0.001f;
    
    double f = 2000.0 * freq; // Original scales by 2000 Hz
    double omega = 2.0 * juce::MathConstants<double>::pi * f / currentSampleRate;
    double cos_omega = std::cos(omega);
    
    voices[voiceIdx].gamma = static_cast<float>(-cos_omega);
    
    // SetDuration - exactly as in original implementation
    float durationScaled = duration * 3.0f * static_cast<float>(currentSampleRate / 44100.0);
    voices[voiceIdx].beta = (durationScaled * 8000.0f - 1.0f) / (durationScaled * 8000.0f + 1.0f);
    
    // Constrain beta for stability
    if (voices[voiceIdx].beta >= 1.0f)
        voices[voiceIdx].beta = 0.999f;
    
    // Update alpha based on gamma and beta - exactly as in original
    voices[voiceIdx].alpha = voices[voiceIdx].gamma * (1.0f + voices[voiceIdx].beta);
    
    // SetAmp - exactly as in original implementation
    voices[voiceIdx].amplitude = amp;
    // This is critical for sound production - sets the gain based on amplitude and beta
    voices[voiceIdx].gain = amp * 0.4f / (1.0f - voices[voiceIdx].beta);
    
    // For debugging - ensure gain is not zero
    if (voices[voiceIdx].gain < 0.001f)
        voices[voiceIdx].gain = 0.001f;
    
    // SetBalance - exactly as in original implementation
    voices[voiceIdx].balance = balance;
    
    // SetNoise - exactly as in original implementation
    voices[voiceIdx].noiseAmount = noise;
    
    // SetDistortion - exactly as in original implementation
    float dist = distortion * 10.0f;
    voices[voiceIdx].d1 = dist;
    
    if (dist > 1.0f)
        voices[voiceIdx].d2 = (1.0f + dist) * (1.0f - 0.8f * (dist - 1.0f) / 19.0f); // empirical hack from original
    else
        voices[voiceIdx].d2 = 1.0f + dist;
    
    // Don't reset filter state during parameter updates
    // This would prevent sound from occurring
    // Only reset when explicitly creating a new voice
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

// No longer needed - we directly implement the original processing algorithm in processBlock

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
    return true; // This plugin requires MIDI input
}

bool OnePingOnlyProcessor::producesMidi() const
{
    return false;
}

bool OnePingOnlyProcessor::isMidiEffect() const
{
    return true; // Explicitly mark as MIDI effect to ensure we get MIDI
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

void OnePingOnlyProcessor::valueTreePropertyChanged(juce::ValueTree& /*tree*/, const juce::Identifier& property)
{
    // Get the parameter ID as a string
    juce::String paramId = property.toString();
    
    // Check if the parameter is a global parameter
    if (paramId == delayTimeID || paramId == feedbackID || paramId == masterVolumeID)
    {
        // For global parameters, update all voices
        updateAllVoicesFromParameters();
        return;
    }
    
    // If it's a voice parameter, determine which voice is affected
    // Try to extract a voice index from the parameter ID
    // Voice parameter IDs are formatted as "base" + voiceIndex (e.g., "freq42")
    
    // First, find which base name matches our parameter
    static const juce::StringArray baseNames = {"freq", "duration", "amp", "balance", "noise", "distortion"};
    
    for (const auto& baseName : baseNames)
    {
        if (paramId.startsWith(baseName))
        {
            // Extract the numeric part (voice index) from the parameter ID
            juce::String indexPart = paramId.substring(baseName.length());
            int voiceIndex = indexPart.getIntValue();
            
            if (voiceIndex >= 0 && voiceIndex < NUM_VOICES)
            {
                // Update just this specific voice
                updateVoiceFromParameters(voiceIndex);
                
                // For debugging
                DBG("Updated parameters for voice " + juce::String(voiceIndex));
                return;
            }
        }
    }
    
    // If we couldn't determine a specific voice, update all voices to be safe
    updateAllVoicesFromParameters();
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OnePingOnlyProcessor();
}