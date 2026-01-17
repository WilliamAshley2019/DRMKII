#include "PluginEditor.h"

//==============================================================================
// SmallLcdDisplay Implementation
//==============================================================================
SmallLcdDisplay::SmallLcdDisplay() {
    setSize(100, 40);
}

void SmallLcdDisplay::paint(juce::Graphics& g) {
    // LCD background (vintage green backlight)
    g.fillAll(juce::Colour(120, 140, 100));
    g.setColour(juce::Colour(60, 70, 50));
    g.drawRect(getLocalBounds(), 2);

    // Dark text on green LCD
    g.setColour(juce::Colour(20, 25, 15));

    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::bold));
    g.drawText(label, 2, 2, getWidth() - 4, 14, juce::Justification::centred, false);

    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 12.0f, juce::Font::plain));
    g.drawText(valueText, 2, 18, getWidth() - 4, 18, juce::Justification::centred, false);
}

void SmallLcdDisplay::setLabel(const juce::String& text) {
    label = text;
    repaint();
}

void SmallLcdDisplay::setValue(const juce::String& text) {
    if (valueText != text) {
        valueText = text;
        repaint();
    }
}

//==============================================================================
// MainLcdDisplay Implementation
//==============================================================================
MainLcdDisplay::MainLcdDisplay() {
    setSize(500, 80);
}

void MainLcdDisplay::paint(juce::Graphics& g) {
    // Vintage green LCD background
    g.fillAll(juce::Colour(120, 140, 100));
    g.setColour(juce::Colour(60, 70, 50));
    g.drawRect(getLocalBounds(), 2);

    // Display text
    g.setColour(juce::Colour(20, 25, 15));

    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 16.0f, juce::Font::bold));

    for (int i = 0; i < 4; ++i) {
        g.drawText(lines[i], 5, 2 + i * 18, getWidth() - 10, 18, juce::Justification::left, false);
    }
}

void MainLcdDisplay::setText(const juce::String& text, int line) {
    if (line >= 0 && line < 4 && lines[line] != text) {
        lines[line] = text;
        repaint();
    }
}

//==============================================================================
// ParameterKnobWithLcd Implementation
//==============================================================================
ParameterKnobWithLcd::ParameterKnobWithLcd() : parameter(nullptr) {
    knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    knob.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
        juce::MathConstants<float>::pi * 2.75f, true);
    knob.setLookAndFeel(&blackMetalLNF);
    knob.setVelocityBasedMode(false);
    knob.setRange(0.0, 1.0, 0.01);
    knob.setValue(0.5);
    knob.setDoubleClickReturnValue(true, 0.5);
    knob.onValueChange = [this] { updateLcdValue(); };

    addAndMakeVisible(knob);
    addAndMakeVisible(lcd);
    lcd.setLabel("PARAM");
    lcd.setValue("0.50");
}

ParameterKnobWithLcd::~ParameterKnobWithLcd() {
    knob.setLookAndFeel(nullptr);
}

void ParameterKnobWithLcd::attachParameter(juce::AudioProcessorValueTreeState& apvts,
    const juce::String& paramID,
    const juce::String& lbl,
    const juce::String& unit) {
    parameter = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(paramID));
    paramLabel = lbl;
    paramUnit = unit;

    if (parameter != nullptr) {
        knob.setRange(parameter->range.start,
            parameter->range.end,
            parameter->range.interval);
        knob.setValue(parameter->get(), juce::dontSendNotification);
    }

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, paramID, knob);

    lcd.setLabel(lbl);
    updateLcdValue();
}

void ParameterKnobWithLcd::resized() {
    auto area = getLocalBounds();
    const int lcdHeight = 40;
    const int knobHeight = area.getHeight() - lcdHeight - 5;

    auto knobArea = area.removeFromTop(knobHeight).reduced(5);
    int knobSize = juce::jmin(knobArea.getWidth(), knobArea.getHeight());
    knob.setBounds(knobArea.withSizeKeepingCentre(knobSize, knobSize));

    area.removeFromTop(5);
    lcd.setBounds(area.withSizeKeepingCentre(90, 40));
}

void ParameterKnobWithLcd::updateDisplay() {
    updateLcdValue();
}

void ParameterKnobWithLcd::updateLcdValue() {
    if (!parameter) return;

    const float value = knob.getValue();
    juce::String display;

    if (paramUnit == "s") display = juce::String(value, 1) + "s";
    else if (paramUnit == "ms") display = juce::String(value, 0) + "ms";
    else if (paramUnit == "%") display = juce::String(value * 100, 0) + "%";
    else if (paramUnit == "x") display = juce::String(value, 2) + "x";
    else display = juce::String(value, 2);

    lcd.setValue(display);
}

//==============================================================================
// MixSliderWithLcd Implementation
//==============================================================================
MixSliderWithLcd::MixSliderWithLcd() {
    // Configure slider
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setLookAndFeel(&blackMetalSliderLNF);
    slider.setRange(0.0, 1.0, 0.01);
    slider.setValue(0.5);
    slider.setScrollWheelEnabled(true);
    slider.setDoubleClickReturnValue(true, 0.5);
    slider.addListener(this);

    // Configure label
    label.setText("DRY/WET MIX", juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, juce::Colour(200, 200, 200));

    // Configure LCD display
    lcdDisplay.setValueText("50%");

    addAndMakeVisible(slider);
    addAndMakeVisible(label);
    addAndMakeVisible(lcdDisplay);
}

MixSliderWithLcd::~MixSliderWithLcd() {
    slider.setLookAndFeel(nullptr);
    slider.removeListener(this);
}

void MixSliderWithLcd::attachParameter(juce::AudioProcessorValueTreeState& apvts,
    const juce::String& paramID) {
    auto* param = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(paramID));
    if (param != nullptr) {
        slider.setRange(param->range.start,
            param->range.end,
            param->range.interval);
        slider.setValue(param->get(), juce::dontSendNotification);
    }

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, paramID, slider);

    updateDisplay();
}

void MixSliderWithLcd::resized() {
    auto area = getLocalBounds();

    // Label at top
    label.setBounds(area.removeFromTop(20));

    // Slider in middle
    slider.setBounds(area.removeFromTop(25));

    // LCD display at bottom (stretches full width)
    lcdDisplay.setBounds(area);
}

void MixSliderWithLcd::sliderValueChanged(juce::Slider* changedSlider) {
    if (changedSlider == &slider) {
        updateDisplay();
    }
}

void MixSliderWithLcd::updateDisplay() {
    const float value = slider.getValue();  // 0.0 to 1.0

    // Update LCD block visualizer (0.0-1.0 range)
    lcdDisplay.setValue(value);

    // Generate display text (percentage)
    int wetPercent = static_cast<int>(value * 100.0f);

    juce::String displayText;

    // Simple percentage display
    if (value < 0.01f) {
        displayText = "DRY";
    }
    else if (value > 0.99f) {
        displayText = "WET";
    }
    else {
        displayText = juce::String(wetPercent) + "%";
    }

    lcdDisplay.setValueText(displayText);
}

//==============================================================================
// DSP256XLReverbEditor Implementation
//==============================================================================
DSP256XLReverbEditor::DSP256XLReverbEditor(DSP256XLReverbProcessor& p)
    : AudioProcessorEditor(&p), processor(p) {

    setSize(1000, 700);
    setResizable(true, true);
    setResizeLimits(800, 550, 1200, 800);

    // Setup main LCD
    addAndMakeVisible(mainLcd);
    mainLcd.setText("DSP-256XL DIGITAL REVERB", 0);
    mainLcd.setText("Schroeder Architecture", 1);
    mainLcd.setText("8 Combs + 4 Allpasses", 2);
    mainLcd.setText("All Parameters Active", 3);

    // Setup mix slider with LCD
    addAndMakeVisible(mixSlider);
    mixSlider.attachParameter(processor.getAPVTS(), "mix");

    // Create all knobs
    createKnobs();

    // Call resized() immediately
    resized();
}

DSP256XLReverbEditor::~DSP256XLReverbEditor() {
    for (auto& knob : knobs) {
        if (knob) {
            knob.reset();
        }
    }
}

void DSP256XLReverbEditor::createKnobs() {
    // 3 rows, 5 knobs each = 15 parameters total
    struct ParamInfo {
        juce::String id, label, unit;
    };

    std::vector<ParamInfo> params = {
        // Row 1: Core Reverb Parameters
        {"decay", "DECAY", "s"},
        {"predelay", "PRE-DLY", "ms"},
        {"damping", "DAMP", "%"},
        {"diffusion", "DIFFUSE", "%"},
        {"revdiff", "RV-DIFF", "%"},

        // Row 2: Room & Space
        {"size", "SIZE", "%"},
        {"volume", "VOL", "x"},
        {"early", "EARLY", "%"},
        {"reflect", "REFLE", "%"},
        {"position", "POS", "%"},

        // Row 3: Tail & Tone
        {"refdelay", "REF-DLY", "x"},
        {"subdelay", "TAIL-DLY", "x"},
        {"sublevel", "TAIL", "%"},
        {"envelop", "ENVLP", "%"},
        {"tielevel", "HF", "%"}
    };

    for (size_t i = 0; i < params.size(); ++i) {
        knobs[i] = std::make_unique<ParameterKnobWithLcd>();
        knobs[i]->attachParameter(processor.getAPVTS(),
            params[i].id,
            params[i].label,
            params[i].unit);
        addAndMakeVisible(*knobs[i]);
    }
}

void DSP256XLReverbEditor::paint(juce::Graphics& g) {
    // Brushed aluminum background
    juce::ColourGradient bgGrad(
        juce::Colour(85, 85, 90), 0.0f, 0.0f,
        juce::Colour(55, 55, 60), 0.0f, (float)getHeight(), false);
    g.setGradientFill(bgGrad);
    g.fillAll();

    // Dark panel sections
    g.setColour(juce::Colour(25, 25, 30));
    g.fillRect(0, 0, getWidth(), 120);

    // Title
    g.setColour(juce::Colour(180, 180, 185));
    g.setFont(juce::FontOptions("Arial", 24.0f, juce::Font::bold));
    g.drawText("WXYZ DRMKII", 0, 20, getWidth(), 30, juce::Justification::centred);

    g.setFont(juce::FontOptions("Arial", 12.0f, juce::Font::plain));
    g.drawText("DIGITAL REVERB", 0, 50, getWidth(), 20, juce::Justification::centred);

    // Section divider under mix slider area
    g.setColour(juce::Colour(100, 100, 105));
    g.drawLine(20.0f, 260.0f, (float)(getWidth() - 20), 260.0f, 2.0f);

    // Bottom info
    g.setColour(juce::Colour(140, 140, 145));
    g.setFont(juce::FontOptions(10.0f, juce::Font::plain));
    g.drawText("Schroeder Algorithm | 8 Parallel Combs + 4 Series Allpasses",
        0, getHeight() - 30, getWidth(), 20, juce::Justification::centred);
}

void DSP256XLReverbEditor::resized() {
    auto area = getLocalBounds();

    // Title area
    area.removeFromTop(80);

    // Main LCD
    auto lcdArea = area.removeFromTop(90);
    mainLcd.setBounds(lcdArea.reduced(50, 5));

    // Spacing
    area.removeFromTop(10);

    // Mix slider section (with LCD) - Adjusted height
    auto mixArea = area.removeFromTop(70);  // Enough for label + slider + LCD
    mixSlider.setBounds(mixArea.reduced(80, 5));

    // Spacing under mix slider
    area.removeFromTop(15);

    // 3 rows of knobs
    const int rowHeight = 160;
    const int rowSpacing = 15;
    const int margin = 30;

    // Row 1: 5 knobs
    auto row1 = area.removeFromTop(rowHeight);
    layoutKnobRow(row1, 0, 5, margin);
    area.removeFromTop(rowSpacing);

    // Row 2: 5 knobs
    auto row2 = area.removeFromTop(rowHeight);
    layoutKnobRow(row2, 5, 5, margin);
    area.removeFromTop(rowSpacing);

    // Row 3: 5 knobs
    auto row3 = area.removeFromTop(rowHeight);
    layoutKnobRow(row3, 10, 5, margin);
}

void DSP256XLReverbEditor::layoutKnobRow(juce::Rectangle<int> area, int startIdx, int count, int margin) {
    const int totalWidth = getWidth() - 2 * margin;
    const int knobWidth = totalWidth / count;
    const int spacing = 5;

    for (int i = 0; i < count; ++i) {
        int idx = startIdx + i;
        if (idx < 15 && knobs[idx]) {
            int x = margin + i * knobWidth;
            if (i > 0) x += spacing;
            knobs[idx]->setBounds(x, area.getY(), knobWidth - spacing, area.getHeight());
        }
    }
}