#pragma once
#include <juce_graphics/juce_graphics.h>

namespace Theme {
    namespace Colors {
        inline const juce::Colour background      { 0xff16161e };
        inline const juce::Colour track           { 0xff2a2a3a };
        inline const juce::Colour innerCircle     { 0xff252535 };
        inline const juce::Colour border          { 0xff2e2e40 };
        
        // Text colors
        inline const juce::Colour textPrimary     { 0xffe8e8f0 };
        inline const juce::Colour textSecondary   { 0xff8888a0 };
        inline const juce::Colour textSecondaryDim{ 0xcc8888a0 };
        inline const juce::Colour textValue       { 0xffffffff };

        // Accent colors
        inline const juce::Colour globalAccent    { 0xfff0c040 };
        inline const juce::Colour speedAccent     { 0xffe05050 };
        inline const juce::Colour reverseAccent   { 0xff40c8c8 };
        inline const juce::Colour repeatAccent    { 0xffa060e0 };
    } // namespace Colors

    namespace Metrics {
        constexpr int   windowWidth    = 640;
        constexpr int   windowHeight   = 400;
        constexpr int   headerHeight   = 56;
        constexpr float knobRadius     = 34.0f;
        constexpr float innerRadius    = 24.0f;
        constexpr float trackThickness = 6.0f;
        constexpr float rotaryStartAngle = juce::MathConstants<float>::pi * 1.25f;
        constexpr float rotaryEndAngle = juce::MathConstants<float>::pi * 2.75f;
    }
}