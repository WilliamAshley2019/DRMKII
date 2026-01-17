// BlackMetalSliderLNF.h
#pragma once
#include <JuceHeader.h>

// =======================================
// Black Metal Slider LookAndFeel (Horizontal)
// =======================================
class BlackMetalSliderLNF : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider(juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        const juce::Slider::SliderStyle,
        juce::Slider&) override
    {
        auto bounds = juce::Rectangle<float>(
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(width),
            static_cast<float>(height)).reduced(2.0f);

        const float sliderHeight = bounds.getHeight() * 0.6f;
        const float trackHeight = sliderHeight * 0.3f;
        const float centerY = bounds.getCentreY();

        // Draw track background
        g.setColour(juce::Colour(15, 15, 15));
        g.fillRoundedRectangle(bounds.getX(), centerY - trackHeight * 0.5f,
            bounds.getWidth(), trackHeight, trackHeight * 0.5f);

        // Draw track foreground (filled portion)
        const float fillWidth = sliderPos * bounds.getWidth();
        if (fillWidth > 0)
        {
            juce::ColourGradient grad(
                juce::Colour(60, 60, 60),
                bounds.getX(), centerY,
                juce::Colour(25, 25, 25),
                bounds.getX() + fillWidth, centerY,
                false);

            g.setGradientFill(grad);
            g.fillRoundedRectangle(bounds.getX(), centerY - trackHeight * 0.5f,
                fillWidth, trackHeight, trackHeight * 0.5f);
        }

        // Draw slider thumb
        const float thumbWidth = sliderHeight * 0.8f;
        const float thumbHeight = sliderHeight * 1.2f;
        const float thumbX = bounds.getX() + sliderPos * bounds.getWidth() - thumbWidth * 0.5f;
        const float thumbY = centerY - thumbHeight * 0.5f;

        juce::ColourGradient thumbGrad(
            juce::Colour(45, 45, 45),
            thumbX + thumbWidth * 0.5f, thumbY,
            juce::Colour(10, 10, 10),
            thumbX + thumbWidth * 0.5f, thumbY + thumbHeight,
            false);

        g.setGradientFill(thumbGrad);
        g.fillRoundedRectangle(thumbX, thumbY, thumbWidth, thumbHeight, 2.0f);

        // Draw thumb outline
        g.setColour(juce::Colour(80, 80, 80));
        g.drawRoundedRectangle(thumbX, thumbY, thumbWidth, thumbHeight, 2.0f, 1.0f);

        // Draw center indicator line
        g.setColour(juce::Colour(100, 100, 100));
        g.drawLine(bounds.getCentreX(), thumbY,
            bounds.getCentreX(), thumbY + thumbHeight, 0.5f);
    }
};