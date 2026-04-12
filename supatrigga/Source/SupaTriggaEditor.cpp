#include "SupaTriggaEditor.h"
#include "Theme.h"

SupaTriggaLookAndFeel::SupaTriggaLookAndFeel() {
  
  // Configure default colors
  setColour(juce::Slider::rotarySliderFillColourId, Theme::Colors::globalAccent);
  setColour(juce::Slider::rotarySliderOutlineColourId, Theme::Colors::track);
  setColour(juce::Slider::textBoxTextColourId, Theme::Colors::textValue);

#if JUCE_MAC
  interFont = juce::Typeface::createSystemTypefaceFor(juce::FontOptions{}.withName("Inter").withHeight(12.0f).withStyle("Bold"));
#endif
}

void SupaTriggaLookAndFeel::drawRotarySlider(juce::Graphics &g, int x, int y,
                                             int width, int height,
                                             float sliderPos,
                                             const float rotaryStartAngle,
                                             const float rotaryEndAngle,
                                             juce::Slider &slider) {
  const float radius = 34.0f;
  const float centreX = (float)x + (float)width * 0.5f;
  const float centreY = (float)y + (float)height * 0.5f;

  // Track
  const float trackThickness = 6.0f;
  g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
  juce::Path trackPath;
  trackPath.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
  g.strokePath(trackPath, juce::PathStrokeType(trackThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

  // Fill
  auto fillColour = slider.findColour(juce::Slider::rotarySliderFillColourId);
  g.setColour(fillColour);

  // Only draw fill if greater than 0
  if (sliderPos > 0.0f) {
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    juce::Path fillPath;
    fillPath.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
    g.strokePath(fillPath, juce::PathStrokeType(trackThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
  }

  // Center Inner Circle
  const float innerRadius = Theme::Metrics::innerRadius;
  const float innerDiameter = innerRadius * 2.0f;
  g.setColour(Theme::Colors::background);
  g.fillEllipse(centreX - innerRadius, centreY - innerRadius, innerDiameter, innerDiameter);
  g.setColour(Theme::Colors::innerCircle);
  g.drawEllipse(centreX - innerRadius, centreY - innerRadius, innerDiameter, innerDiameter, 1.0f);

  // Text Value
  g.setColour(slider.findColour(juce::Slider::textBoxTextColourId));

  auto font = juce::Font(juce::FontOptions{}.withHeight(12.0f).withStyle("Bold"));
  if (interFont != nullptr) {
    font = juce::Font(juce::FontOptions(interFont).withHeight(12.0f));
  }
  g.setFont(font);

  juce::String text = slider.getTextFromValue(slider.getValue());
  g.drawText(text, x, y, width, height, juce::Justification::centred, false);
}

void SupaTriggaLookAndFeel::drawToggleButton(
    juce::Graphics &g, juce::ToggleButton &button,
    bool /*shouldDrawButtonAsHighlighted*/,
    bool /*shouldDrawButtonAsDown*/) {
  const float radius = 34.0f;
  const float centreX = button.getLocalBounds().getCentreX();
  const float centreY = button.getLocalBounds().getCentreY();

  // Toggle Track
  g.setColour(Theme::Colors::track);
  g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 6.0f);

  // Toggle Fill (ON/OFF)
  if (button.getToggleState()) {
    g.setColour(button.findColour(juce::ToggleButton::tickColourId));
    g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 6.0f);
  }

  // Center Inner Circle
  const float innerRadius = Theme::Metrics::innerRadius;
  const float innerDiameter = innerRadius * 2.0f;
  g.setColour(Theme::Colors::background);
  g.fillEllipse(centreX - innerRadius, centreY - innerRadius, innerDiameter, innerDiameter);
  g.setColour(Theme::Colors::innerCircle);
  g.drawEllipse(centreX - innerRadius, centreY - innerRadius, innerDiameter, innerDiameter, 1.0f);

  // Text Label
  g.setColour(button.getToggleState()
                  ? Theme::Colors::textValue
                  : Theme::Colors::textSecondary);

  auto font = juce::Font(juce::FontOptions{}.withHeight(12.0f).withStyle("Bold"));
  if (interFont != nullptr) {
    font = juce::Font(juce::FontOptions(interFont).withHeight(12.0f));
  }
  g.setFont(font);

  g.drawText(button.getName(), button.getLocalBounds(), juce::Justification::centred, false);
}

SupaTriggaEditor::SupaTriggaEditor(SupaTriggaProcessor &p,
                                   juce::AudioProcessorValueTreeState &vts)
    : AudioProcessorEditor(&p), audioProcessor(p), apvts(vts) {
  setLookAndFeel(&customLookAndFeel);

  // Global section
  setupKnob(rearrangeKnob, "global");
  rearrangeAttach = std::make_unique<SliderAttachment>(apvts, SupaTriggaProcessor::PROB_REARRANGE_ID, rearrangeKnob);
  
  setupKnob(slicesKnob, "global");
  slicesAttach = std::make_unique<SliderAttachment>(apvts, SupaTriggaProcessor::GRANULARITY_ID, slicesKnob);
  
  setupKnob(silenceKnob, "global");
  silenceAttach = std::make_unique<SliderAttachment>(apvts, SupaTriggaProcessor::PROB_SILENCE_ID, silenceKnob);

  // Brake section
  setupKnob(brakeProbKnob, "speed");
  brakeProbAttach = std::make_unique<SliderAttachment>(apvts, SupaTriggaProcessor::PROB_SPEED_ID, brakeProbKnob);
  
  setupKnob(brakeTimeKnob, "speed");
  brakeTimeAttach = std::make_unique<SliderAttachment>(apvts, SupaTriggaProcessor::SPEED_ID, brakeTimeKnob);
  
  setupToggle(brakeInstantToggle, "speed");
  brakeInstantAttach = std::make_unique<ButtonAttachment>(apvts, SupaTriggaProcessor::INSTANT_SPEED_ID, brakeInstantToggle);

  // Reverse section
  setupKnob(reverseProbKnob, "reverse");
  reverseProbAttach = std::make_unique<SliderAttachment>(apvts, SupaTriggaProcessor::PROB_REVERSE_ID, reverseProbKnob);
  
  setupToggle(reverseInstantToggle, "reverse");
  reverseInstantAttach = std::make_unique<ButtonAttachment>(apvts, SupaTriggaProcessor::INSTANT_REVERSE_ID, reverseInstantToggle);

  // Repeat section
  setupKnob(repeatProbKnob, "repeat");
  repeatProbAttach = std::make_unique<SliderAttachment>(apvts, SupaTriggaProcessor::PROB_REPEAT_ID, repeatProbKnob);
  
  setupToggle(repeatInstantToggle, "repeat");
  repeatInstantAttach = std::make_unique<ButtonAttachment>(apvts, SupaTriggaProcessor::INSTANT_REPEAT_ID, repeatInstantToggle);

  // Make window non-resizable
  setResizable(false, false);

  // Set Window Size 
  setSize(Theme::Metrics::windowWidth, Theme::Metrics::windowHeight);
}

SupaTriggaEditor::~SupaTriggaEditor() { setLookAndFeel(nullptr); }

void SupaTriggaEditor::setupKnob(juce::Slider &slider,
                                 const juce::String &styleClass) {
  slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0); // Text is drawn by LookAndFeel
  slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
  addAndMakeVisible(slider);

  // Set styling colors
  if (styleClass == "global") {
    slider.setColour(juce::Slider::rotarySliderFillColourId, Theme::Colors::globalAccent);
  } else if (styleClass == "speed") {
    slider.setColour(juce::Slider::rotarySliderFillColourId, Theme::Colors::speedAccent);
  } else if (styleClass == "reverse") {
    slider.setColour(juce::Slider::rotarySliderFillColourId, Theme::Colors::reverseAccent);
  } else if (styleClass == "repeat") {
    slider.setColour(juce::Slider::rotarySliderFillColourId, Theme::Colors::repeatAccent);
  }
}

void SupaTriggaEditor::setupToggle(juce::ToggleButton &toggle,
                                   const juce::String &styleClass) {
  // Configure text change on toggle state change using a listener
  toggle.setName(toggle.getToggleState() ? "On" : "Off");
  toggle.onClick = [&toggle] {
    toggle.setName(toggle.getToggleState() ? "On" : "Off");
  };
  addAndMakeVisible(toggle);

  if (styleClass == "global") {
    toggle.setColour(juce::ToggleButton::tickColourId, Theme::Colors::globalAccent);
  } else if (styleClass == "speed") {
    toggle.setColour(juce::ToggleButton::tickColourId, Theme::Colors::speedAccent);
  } else if (styleClass == "reverse") {
    toggle.setColour(juce::ToggleButton::tickColourId, Theme::Colors::reverseAccent);
  } else if (styleClass == "repeat") {
    toggle.setColour(juce::ToggleButton::tickColourId, Theme::Colors::repeatAccent);
  }
}

void SupaTriggaEditor::paint(juce::Graphics &g) {
  // Background
  g.fillAll(Theme::Colors::background);

  // Header Panel is flush with the background

  // Draw thick bottom line of header
  g.setColour(Theme::Colors::border);
  g.drawLine(0.0f, 56.0f, (float)getWidth(), 56.0f, 1.0f);

  // Draw header text
  g.setColour(Theme::Colors::textPrimary);
  g.setFont(juce::FontOptions(20.0f).withStyle("Bold"));
  g.drawText("SUPATRIGGA", 20, 0, 200, 56, juce::Justification::centredLeft, true);

  g.setColour(Theme::Colors::textSecondaryDim);
  g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
  g.drawText("BY SMARTELECTRONIX", getWidth() - 200, 0, 180, 56, juce::Justification::centredRight, true);

  // Section Dividers (the cross in the grid)
  auto gridBounds = getLocalBounds().withTrimmedTop(56);
  int midX = gridBounds.getCentreX();
  int midY = gridBounds.getCentreY();

  g.setColour(Theme::Colors::border);
  // Vertical line (indented top and bottom by 16px)
  g.drawLine((float)midX, (float)(gridBounds.getY() + 16), (float)midX, (float)(gridBounds.getBottom() - 16), 1.0f);
  // Horizontal line (indented left/right by 20px, gap in center)
  g.drawLine(20.0f, (float)midY, (float)(midX - 16), (float)midY, 1.0f);
  g.drawLine((float)(midX + 16), (float)midY, (float)(getWidth() - 20), (float)midY, 1.0f);

  // Draw Section Titles
  auto drawSectionTitle = [&](const juce::String &title, juce::Colour c, int x, int y) {
    g.setColour(c);
    g.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    g.drawText(title, x + 20, y + 16, 100, 20, juce::Justification::topLeft, true);
  };

  drawSectionTitle("GLOBAL", Theme::Colors::globalAccent, gridBounds.getX(), gridBounds.getY());
  drawSectionTitle("BRAKE", Theme::Colors::speedAccent, midX, gridBounds.getY());
  drawSectionTitle("REVERSE", Theme::Colors::reverseAccent, gridBounds.getX(), midY);
  drawSectionTitle("REPEAT", Theme::Colors::repeatAccent, midX, midY);

  // Draw Sub-labels for controls
  auto drawLabel = [&](const juce::String &text, juce::Component &comp) {
    g.setColour(Theme::Colors::textSecondary);
    g.setFont( juce::FontOptions(11.0f).withStyle("Bold"));
    auto bounds = comp.getBounds();
    // Move the text up by drawing it in a box higher above the component
    g.drawText(text, bounds.getX() - 20, bounds.getY() - 28, bounds.getWidth() + 40, 20, juce::Justification::centredBottom, false);
  };

  // Global Section Label centers
  drawLabel("REARRANGE", rearrangeKnob);
  drawLabel("SLICES", slicesKnob);
  drawLabel("SILENCE", silenceKnob);

  // Brake Section Label centers
  drawLabel("BRAKE PROB", brakeProbKnob);
  drawLabel("BRAKE TIME", brakeTimeKnob);
  drawLabel("INSTANT", brakeInstantToggle);

  // Reverse Section Label centers
  drawLabel("REVERSE PROB", reverseProbKnob);
  drawLabel("INSTANT", reverseInstantToggle);

  // Repeat Section Label centers
  drawLabel("REPEAT PROB", repeatProbKnob);
  drawLabel("INSTANT", repeatInstantToggle);
}

void SupaTriggaEditor::resized() {
  auto gridBounds = getLocalBounds().withTrimmedTop(56);

  int midX = gridBounds.getWidth() / 2;
  int midY = gridBounds.getHeight() / 2;

  auto globalRect = gridBounds.withWidth(midX).withHeight(midY);
  auto speedRect = globalRect.withX(midX);
  auto reverseRect = globalRect.withY(gridBounds.getY() + midY);
  auto repeatRect = reverseRect.withX(midX);

  // Controls Row bounds (leave room for title and labels)
  auto globalRow = globalRect.withTrimmedTop(40).withTrimmedBottom(18);
  auto speedRow = speedRect.withTrimmedTop(40).withTrimmedBottom(18);
  auto reverseRow = reverseRect.withTrimmedTop(40).withTrimmedBottom(18);
  auto repeatRow = repeatRect.withTrimmedTop(40).withTrimmedBottom(18);

  // juce::FlexBox is ideal here to distribute spacing
  auto buildFlex = [](juce::FlexBox &fb) {
    fb.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    fb.alignItems = juce::FlexBox::AlignItems::flexEnd; // Bottom align to fix margin above divider
    fb.flexDirection = juce::FlexBox::Direction::row;
  };

  auto mapItem = [](juce::Component &c) {
    return juce::FlexItem(c).withWidth(80).withHeight(80).withMargin(
        juce::FlexItem::Margin(0, 4, 0, 4));
  };

  {
    juce::FlexBox fb;
    buildFlex(fb);
    fb.items.add(mapItem(rearrangeKnob));
    fb.items.add(mapItem(slicesKnob));
    fb.items.add(mapItem(silenceKnob));
    fb.performLayout(globalRow);
  }

  {
    juce::FlexBox fb;
    buildFlex(fb);
    fb.items.add(mapItem(brakeProbKnob));
    fb.items.add(mapItem(brakeTimeKnob));
    fb.items.add(mapItem(brakeInstantToggle));
    fb.performLayout(speedRow);
  }

  {
    juce::FlexBox fb;
    buildFlex(fb);
    fb.items.add(mapItem(reverseProbKnob));
    fb.items.add(mapItem(reverseInstantToggle));
    fb.performLayout(reverseRow);
  }

  {
    juce::FlexBox fb;
    buildFlex(fb);
    fb.items.add(mapItem(repeatProbKnob));
    fb.items.add(mapItem(repeatInstantToggle));
    fb.performLayout(repeatRow);
  }
}