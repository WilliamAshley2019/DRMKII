// BlackMetalKnobLNF.h
#pragma once
#include <JuceHeader.h>

// =======================================
// Black Metal Knob LookAndFeel
// =======================================
class BlackMetalKnobLNF : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g,
        int x, int y, int w, int h,
        float sliderPos,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider&) override
    {
        auto bounds = juce::Rectangle<float>(
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(w),
            static_cast<float>(h)).reduced(6.0f);

        const float radius = bounds.getWidth() * 0.5f;
        const float angle = rotaryStartAngle
            + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Outer ring
        g.setColour(juce::Colour(15, 15, 15));
        g.fillEllipse(bounds);

        // Inner metal gradient
        juce::ColourGradient grad(
            juce::Colour(45, 45, 45),
            bounds.getCentreX(), bounds.getY(),
            juce::Colour(10, 10, 10),
            bounds.getCentreX(), bounds.getBottom(),
            false);

        g.setGradientFill(grad);
        g.fillEllipse(bounds.reduced(radius * 0.15f));

        // Indicator
        juce::Path p;
        p.addRectangle(-1.5f, -radius * 0.6f, 3.0f, radius * 0.4f);

        g.setColour(juce::Colours::white);
        g.fillPath(p,
            juce::AffineTransform::rotation(angle)
            .translated(bounds.getCentre()));
    }
};