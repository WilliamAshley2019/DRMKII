#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "BlackMetalKnobLNF.h"
#include "BlackMetalSliderLNF.h"
#include "ThinBlockLcdDisplay.h" 

//==============================================================================
// Small LCD Display (Vintage Green)
//==============================================================================
class SmallLcdDisplay : public juce::Component {
public:
    SmallLcdDisplay();
    void paint(juce::Graphics& g) override;
    void setLabel(const juce::String& text);
    void setValue(const juce::String& text);

private:
    juce::String label, valueText;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SmallLcdDisplay)
};

//==============================================================================
// Main LCD Display (Vintage Style)
//==============================================================================
class MainLcdDisplay : public juce::Component {
public:
    MainLcdDisplay();
    void paint(juce::Graphics& g) override;
    void setText(const juce::String& text, int line);

private:
    juce::String lines[4];
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainLcdDisplay)
};

//==============================================================================
// Parameter Knob with LCD
//==============================================================================
class ParameterKnobWithLcd : public juce::Component {
public:
    ParameterKnobWithLcd();
    ~ParameterKnobWithLcd() override;

    void attachParameter(juce::AudioProcessorValueTreeState& apvts,
        const juce::String& paramID,
        const juce::String& lbl,
        const juce::String& unit);

    void resized() override;
    void updateDisplay();

private:
    juce::Slider knob;
    SmallLcdDisplay lcd;
    BlackMetalKnobLNF blackMetalLNF;
    juce::AudioParameterFloat* parameter;
    juce::String paramLabel, paramUnit;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    void updateLcdValue();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterKnobWithLcd)
};

//==============================================================================
// Mix Slider Component WITH LCD Display
//==============================================================================
class MixSliderWithLcd : public juce::Component,
    public juce::Slider::Listener {
public:
    MixSliderWithLcd();
    ~MixSliderWithLcd() override;

    void attachParameter(juce::AudioProcessorValueTreeState& apvts,
        const juce::String& paramID);

    void resized() override;
    void sliderValueChanged(juce::Slider* slider) override;

    void updateDisplay();

private:
    juce::Slider slider;
    BlackMetalSliderLNF blackMetalSliderLNF;
    juce::Label label;
    ThinBlockLcdDisplay lcdDisplay;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixSliderWithLcd)
};

//==============================================================================
// Main Plugin Editor
//==============================================================================
class DSP256XLReverbEditor : public juce::AudioProcessorEditor {
public:
    explicit DSP256XLReverbEditor(DSP256XLReverbProcessor& p);
    ~DSP256XLReverbEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DSP256XLReverbProcessor& processor;
    MainLcdDisplay mainLcd;

    // Mix slider with LCD display
    MixSliderWithLcd mixSlider;

    // 3 rows of knobs
    std::unique_ptr<ParameterKnobWithLcd> knobs[15];

    void layoutKnobRow(juce::Rectangle<int> area, int startIdx, int count, int margin);
    void createKnobs();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DSP256XLReverbEditor)
};