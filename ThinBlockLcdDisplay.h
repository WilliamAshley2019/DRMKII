// ThinBlockLcdDisplay.h
#pragma once
#include <JuceHeader.h>

//==============================================================================
// Thin Block LCD Display (For gain visualization)
//==============================================================================
class ThinBlockLcdDisplay final : public juce::Component
{
public:
    ThinBlockLcdDisplay() { setSize(100, 10); }

    void paint(juce::Graphics& g) override {
        // LCD background
        g.fillAll(juce::Colour(120, 140, 100));
        g.setColour(juce::Colour(60, 70, 50));
        g.drawRect(getLocalBounds(), 1);

        // Draw blocks based on value
        const int totalBlocks = 10;
        const float blockWidth = static_cast<float>(getWidth() - 4) / totalBlocks;
        const float blockHeight = static_cast<float>(getHeight() - 4);

        // Calculate how many blocks to fill based on normalized value (0-1)
        const int blocksToFill = juce::jlimit(0, totalBlocks,
            static_cast<int>(std::round(value * totalBlocks)));

        // Draw filled blocks
        g.setColour(juce::Colour(20, 25, 15));
        for (int i = 0; i < blocksToFill; ++i)
        {
            const float x = 2.0f + i * blockWidth;
            g.fillRect(x, 2.0f, blockWidth - 1.0f, blockHeight);
        }

        // Draw block separators
        g.setColour(juce::Colour(60, 70, 50));
        for (int i = 1; i < totalBlocks; ++i)
        {
            const float x = 2.0f + i * blockWidth;
            g.drawLine(x, 2.0f, x, getHeight() - 2.0f, 0.5f);
        }

        // Draw value text at the end
        g.setColour(juce::Colour(20, 25, 15));
        g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 8.0f, juce::Font::plain));
        g.drawText(valueText, getWidth() - 35, 1, 33, getHeight() - 2, juce::Justification::right, false);
    }

    void setValue(float normalizedValue)
    {
        value = juce::jlimit(0.0f, 1.0f, normalizedValue);
        repaint();
    }

    void setValueText(const juce::String& text)
    {
        valueText = text;
        repaint();
    }

private:
    float value = 0.0f;
    juce::String valueText;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThinBlockLcdDisplay)
};