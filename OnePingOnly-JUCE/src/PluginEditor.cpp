#include "PluginEditor.h"

OnePingOnlyEditor::OnePingOnlyEditor(OnePingOnlyProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Set plugin editor size
    setSize(400, 300);
    
    // Title label
    titleLabel.setText("One Ping Only", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions().withHeight(24.0f)).boldened());
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    
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
    delayTimeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    delayTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    delayTimeSlider.setRange(0.0, 1.0, 0.01);
    delayTimeAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getValueTreeState(), "delayTime", delayTimeSlider));
    addAndMakeVisible(delayTimeSlider);
    
    delayTimeLabel.setText("Delay", juce::dontSendNotification);
    delayTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(delayTimeLabel);
    
    // Feedback slider setup
    feedbackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    feedbackSlider.setRange(0.0, 0.95, 0.01);
    feedbackAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getValueTreeState(), "feedback", feedbackSlider));
    addAndMakeVisible(feedbackSlider);
    
    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    feedbackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(feedbackLabel);
    
    // Master volume slider setup
    masterVolumeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    masterVolumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    masterVolumeSlider.setRange(0.0, 1.0, 0.01);
    masterVolumeAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        processorRef.getValueTreeState(), "masterVolume", masterVolumeSlider));
    addAndMakeVisible(masterVolumeSlider);
    
    masterVolumeLabel.setText("Volume", juce::dontSendNotification);
    masterVolumeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(masterVolumeLabel);
    
    // Start timer to check for program changes
    startTimer(100);
}

OnePingOnlyEditor::~OnePingOnlyEditor()
{
    stopTimer();
}

void OnePingOnlyEditor::paint(juce::Graphics& g)
{
    // Fill the background
    g.fillAll(juce::Colour(40, 40, 60));
    
    // Draw a border around the plugin
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);
}

void OnePingOnlyEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Title at the top
    titleLabel.setBounds(area.removeFromTop(30));
    
    // Program selector below title
    area.removeFromTop(10);
    auto programArea = area.removeFromTop(30);
    programSelector.setBounds(programArea.removeFromRight(150));
    
    // Space before knobs
    area.removeFromTop(20);
    
    // Three knobs in a row
    auto knobArea = area.removeFromTop(120);
    auto knobWidth = knobArea.getWidth() / 3;
    
    auto delayArea = knobArea.removeFromLeft(knobWidth);
    delayTimeSlider.setBounds(delayArea.reduced(10));
    delayTimeLabel.setBounds(delayArea.removeFromBottom(20));
    
    auto feedbackArea = knobArea.removeFromLeft(knobWidth);
    feedbackSlider.setBounds(feedbackArea.reduced(10));
    feedbackLabel.setBounds(feedbackArea.removeFromBottom(20));
    
    auto volumeArea = knobArea;
    masterVolumeSlider.setBounds(volumeArea.reduced(10));
    masterVolumeLabel.setBounds(volumeArea.removeFromBottom(20));
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