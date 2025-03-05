#include "PluginEditor.h"

OnePingOnlyEditor::OnePingOnlyEditor(OnePingOnlyProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Set plugin editor size
    setSize(600, 650);
    
    // Title label
    titleLabel.setText("One Ping Only", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions().withHeight(24.0f)).boldened());
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    
    // Section headers
    globalParamsLabel.setText("GLOBAL PARAMETERS", juce::dontSendNotification);
    globalParamsLabel.setFont(juce::Font(juce::FontOptions().withHeight(16.0f)).boldened());
    globalParamsLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(globalParamsLabel);
    
    noteParamsLabel.setText("NOTE PARAMETERS", juce::dontSendNotification);
    noteParamsLabel.setFont(juce::Font(juce::FontOptions().withHeight(16.0f)).boldened());
    noteParamsLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(noteParamsLabel);
    
    // Program selector setup
    programSelector.setTextWhenNothingSelected("Select a program");
    for (int i = 0; i < p.getNumPrograms(); ++i)
        programSelector.addItem(p.getProgramName(i), i + 1);
    
    programSelector.setSelectedId(p.getCurrentProgram() + 1, juce::dontSendNotification);
    programSelector.onChange = [this] {
        processorRef.setCurrentProgram(programSelector.getSelectedId() - 1);
    };
    addAndMakeVisible(programSelector);
    
    programLabel.setText("Program:", juce::dontSendNotification);
    programLabel.attachToComponent(&programSelector, true);
    addAndMakeVisible(programLabel);
    
    // Delay time slider setup
    delayTimeSlider.setSliderStyle(juce::Slider::LinearVertical);
    delayTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    delayTimeSlider.setRange(0.0, 1.0, 0.01);
    delayTimeAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getValueTreeState(), "delayTime", delayTimeSlider));
    addAndMakeVisible(delayTimeSlider);
    
    delayTimeLabel.setText("Delay", juce::dontSendNotification);
    delayTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(delayTimeLabel);
    
    // Feedback slider setup
    feedbackSlider.setSliderStyle(juce::Slider::LinearVertical);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    feedbackSlider.setRange(0.0, 0.95, 0.01);
    feedbackAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getValueTreeState(), "feedback", feedbackSlider));
    addAndMakeVisible(feedbackSlider);
    
    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    feedbackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(feedbackLabel);
    
    // Master volume slider setup
    masterVolumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    masterVolumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    masterVolumeSlider.setRange(0.0, 1.0, 0.01);
    masterVolumeAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getValueTreeState(), "masterVolume", masterVolumeSlider));
    addAndMakeVisible(masterVolumeSlider);
    
    masterVolumeLabel.setText("Volume", juce::dontSendNotification);
    masterVolumeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(masterVolumeLabel);
    
    // Note selector setup with standard MIDI note mapping
    noteSelector.setTextWhenNothingSelected("Select MIDI Note");
    
    // Define note names (C through B)
    const char* noteNames[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    
    // Create entries for all 128 MIDI notes with correct octave numbering
    for (int midiNote = 0; midiNote < NUM_VOICES; ++midiNote) {
        // Simple formula as requested
        int noteIndex = midiNote % 12;
        int octave = midiNote / 12 - 1;
        
        // Create note name
        juce::String noteName = juce::String(noteNames[noteIndex]) + juce::String(octave);
        
        // Create dropdown item with MIDI note number for clarity
        juce::String displayText = juce::String(midiNote) + ": " + noteName;
        
        // Add to dropdown menu
        noteSelector.addItem(displayText, midiNote + 1);
    }
    
    // Initialize with the note from the processor to maintain state when reopening
    currentNoteIndex = processorRef.currentUISelectedNote;
    noteSelector.setSelectedId(currentNoteIndex + 1, juce::dontSendNotification);
    
    noteSelector.onChange = [this] {
        currentNoteIndex = noteSelector.getSelectedId() - 1;
        // Also update in the processor to maintain state when UI is reopened
        processorRef.currentUISelectedNote = currentNoteIndex;
        
        updateNoteParameterControls();
        
        // Force voice update for this note
        processorRef.updateVoiceFromParameters(currentNoteIndex);
        
        // Make sure the voice is marked as active so parameter changes will be applied
        processorRef.setVoiceActiveState(currentNoteIndex, true);
    };
    addAndMakeVisible(noteSelector);
    
    noteSelectorLabel.setText("MIDI Note:", juce::dontSendNotification);
    noteSelectorLabel.attachToComponent(&noteSelector, true);
    addAndMakeVisible(noteSelectorLabel);
    
    // Set up note parameter sliders
    // Frequency
    frequencySlider.setSliderStyle(juce::Slider::LinearVertical);
    frequencySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    frequencySlider.setRange(0.0, 1.0, 0.01);
    addAndMakeVisible(frequencySlider);
    
    frequencyLabel.setText("Frequency", juce::dontSendNotification);
    frequencyLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(frequencyLabel);
    
    // Duration
    durationSlider.setSliderStyle(juce::Slider::LinearVertical);
    durationSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    durationSlider.setRange(0.0, 1.0, 0.01);
    addAndMakeVisible(durationSlider);
    
    durationLabel.setText("Duration", juce::dontSendNotification);
    durationLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(durationLabel);
    
    // Amplitude
    amplitudeSlider.setSliderStyle(juce::Slider::LinearVertical);
    amplitudeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    amplitudeSlider.setRange(0.0, 1.0, 0.01);
    addAndMakeVisible(amplitudeSlider);
    
    amplitudeLabel.setText("Amplitude", juce::dontSendNotification);
    amplitudeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(amplitudeLabel);
    
    // Balance
    balanceSlider.setSliderStyle(juce::Slider::LinearVertical);
    balanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    balanceSlider.setRange(0.0, 1.0, 0.01);
    addAndMakeVisible(balanceSlider);
    
    balanceLabel.setText("Balance", juce::dontSendNotification);
    balanceLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(balanceLabel);
    
    // Distortion
    distortionSlider.setSliderStyle(juce::Slider::LinearVertical);
    distortionSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    distortionSlider.setRange(0.0, 1.0, 0.01);
    addAndMakeVisible(distortionSlider);
    
    distortionLabel.setText("Distortion", juce::dontSendNotification);
    distortionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(distortionLabel);
    
    // Noise
    noiseSlider.setSliderStyle(juce::Slider::LinearVertical);
    noiseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    noiseSlider.setRange(0.0, 1.0, 0.01);
    addAndMakeVisible(noiseSlider);
    
    noiseLabel.setText("Noise", juce::dontSendNotification);
    noiseLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(noiseLabel);
    
    // Initialize note parameter attachments
    updateNoteParameterControls();
    
    // Make sure the current voice is active (for parameter changes)
    processorRef.setVoiceActiveState(currentNoteIndex, true);
    
    // Start timer to check for program changes
    startTimer(100);
}

OnePingOnlyEditor::~OnePingOnlyEditor()
{
    stopTimer();
}

juce::String OnePingOnlyEditor::createVoiceParamID(int voiceIndex, int paramType)
{
    // This must match the parameter IDs used in the processor
    static juce::String baseNames[NUM_PARAMS_PER_VOICE] = {
        "freq", "duration", "amp", "balance", "noise", "distortion"
    };
    
    return baseNames[paramType] + juce::String(voiceIndex);
}

void OnePingOnlyEditor::updateNoteParameterControls()
{
    // Clear existing attachments
    frequencyAttachment.reset();
    durationAttachment.reset();
    amplitudeAttachment.reset();
    balanceAttachment.reset();
    distortionAttachment.reset();
    noiseAttachment.reset();
    
    // Create new parameter IDs for the selected note
    juce::String freqID = createVoiceParamID(currentNoteIndex, kFreq);
    juce::String durationID = createVoiceParamID(currentNoteIndex, kDuration);
    juce::String ampID = createVoiceParamID(currentNoteIndex, kAmp);
    juce::String balanceID = createVoiceParamID(currentNoteIndex, kBalance);
    juce::String distortionID = createVoiceParamID(currentNoteIndex, kDistortion);
    juce::String noiseID = createVoiceParamID(currentNoteIndex, kNoise);
    
    // Create new attachments for the selected note
    frequencyAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getValueTreeState(), freqID, frequencySlider));
    
    durationAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getValueTreeState(), durationID, durationSlider));
    
    amplitudeAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getValueTreeState(), ampID, amplitudeSlider));
    
    balanceAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getValueTreeState(), balanceID, balanceSlider));
    
    distortionAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getValueTreeState(), distortionID, distortionSlider));
    
    noiseAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getValueTreeState(), noiseID, noiseSlider));
        
    // Make sure to activate the voice for the currently selected note
    processorRef.setVoiceActiveState(currentNoteIndex, true);
    
    // Set up change functions to force parameter updates
    frequencySlider.onValueChange = [this] {
        processorRef.updateVoiceFromParameters(currentNoteIndex);
    };
    
    durationSlider.onValueChange = [this] {
        processorRef.updateVoiceFromParameters(currentNoteIndex);
    };
    
    amplitudeSlider.onValueChange = [this] {
        processorRef.updateVoiceFromParameters(currentNoteIndex);
    };
    
    balanceSlider.onValueChange = [this] {
        processorRef.updateVoiceFromParameters(currentNoteIndex);
    };
    
    distortionSlider.onValueChange = [this] {
        processorRef.updateVoiceFromParameters(currentNoteIndex);
    };
    
    noiseSlider.onValueChange = [this] {
        processorRef.updateVoiceFromParameters(currentNoteIndex);
    };
}

void OnePingOnlyEditor::paint(juce::Graphics& g)
{
    // Fill the background
    g.fillAll(juce::Colour(40, 40, 60));
    
    // Draw a border around the plugin
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);
    
    // Draw separator line between global and note sections
    g.setColour(juce::Colours::grey);
    juce::Rectangle<int> separator(10, 250, getWidth() - 20, 1);
    g.fillRect(separator);
}

void OnePingOnlyEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Title at the top
    titleLabel.setBounds(area.removeFromTop(30));
    
    // Global section
    globalParamsLabel.setBounds(area.removeFromTop(30));
    
    // Program selector
    area.removeFromTop(10);
    auto programArea = area.removeFromTop(30);
    programSelector.setBounds(programArea.removeFromRight(150).withTrimmedLeft(80));
    
    // Space before knobs
    area.removeFromTop(10);
    
    // Global parameter knobs - three in a row
    auto globalKnobArea = area.removeFromTop(120);
    auto knobWidth = globalKnobArea.getWidth() / 3;
    
    auto delayArea = globalKnobArea.removeFromLeft(knobWidth);
    delayTimeSlider.setBounds(delayArea.reduced(10));
    delayTimeLabel.setBounds(delayArea.removeFromBottom(20));
    
    auto feedbackArea = globalKnobArea.removeFromLeft(knobWidth);
    feedbackSlider.setBounds(feedbackArea.reduced(10));
    feedbackLabel.setBounds(feedbackArea.removeFromBottom(20));
    
    auto volumeArea = globalKnobArea;
    masterVolumeSlider.setBounds(volumeArea.reduced(10));
    masterVolumeLabel.setBounds(volumeArea.removeFromBottom(20));
    
    // Add space before note parameters
    area.removeFromTop(40);
    
    // Note parameters section
    noteParamsLabel.setBounds(area.removeFromTop(30));
    
    // Note selector
    area.removeFromTop(10);
    auto noteSelectorArea = area.removeFromTop(30);
    noteSelector.setBounds(noteSelectorArea.removeFromRight(200).withTrimmedLeft(80));
    
    // Space before note parameter knobs
    area.removeFromTop(10);
    
    // Note parameter knobs - first row
    auto noteKnobArea1 = area.removeFromTop(120);
    auto noteKnobWidth = noteKnobArea1.getWidth() / 3;
    
    auto freqArea = noteKnobArea1.removeFromLeft(noteKnobWidth);
    frequencySlider.setBounds(freqArea.reduced(10));
    frequencyLabel.setBounds(freqArea.removeFromBottom(20));
    
    auto durationArea = noteKnobArea1.removeFromLeft(noteKnobWidth);
    durationSlider.setBounds(durationArea.reduced(10));
    durationLabel.setBounds(durationArea.removeFromBottom(20));
    
    auto amplitudeArea = noteKnobArea1;
    amplitudeSlider.setBounds(amplitudeArea.reduced(10));
    amplitudeLabel.setBounds(amplitudeArea.removeFromBottom(20));
    
    // Space between rows
    area.removeFromTop(10);
    
    // Note parameter knobs - second row
    auto noteKnobArea2 = area.removeFromTop(120);
    
    auto balanceArea = noteKnobArea2.removeFromLeft(noteKnobWidth);
    balanceSlider.setBounds(balanceArea.reduced(10));
    balanceLabel.setBounds(balanceArea.removeFromBottom(20));
    
    auto distortionArea = noteKnobArea2.removeFromLeft(noteKnobWidth);
    distortionSlider.setBounds(distortionArea.reduced(10));
    distortionLabel.setBounds(distortionArea.removeFromBottom(20));
    
    auto noiseArea = noteKnobArea2;
    noiseSlider.setBounds(noiseArea.reduced(10));
    noiseLabel.setBounds(noiseArea.removeFromBottom(20));
}

void OnePingOnlyEditor::timerCallback()
{
    // Update program selector if the program has changed in the processor
    int currentProgram = processorRef.getCurrentProgram();
    if (programSelector.getSelectedId() != currentProgram + 1)
    {
        programSelector.setSelectedId(currentProgram + 1, juce::dontSendNotification);
    }
}