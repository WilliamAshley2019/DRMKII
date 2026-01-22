#pragma once
#include <JuceHeader.h>

// One-pole lowpass filter for damping in comb filters
class OnePole {
public:
    OnePole();
    float process(float input, float coeff);
    void reset();
    void clear();

private:
    float state;
};

// Frequency-dependent damping filter for better HF response
class DampingFilter {
public:
    DampingFilter();
    void setCoeffs(float lpCoeff, float hpCoeff);
    float process(float input);
    void clear();

private:
    float lpCoeff, hpCoeff;
    float lpState, hpState;
};

// Enhanced comb filter with frequency-dependent damping
class EnhancedCombFilter {
public:
    EnhancedCombFilter();
    void setSize(int samples);
    float process(float input, float stereoSpread = 0.0f);
    void setDamping(float lpVal, float hpVal);
    void setFeedback(float val);
    void clear();

private:
    std::vector<float> buffer;
    int writeIndex;
    DampingFilter damping;
    float feedback, dampingLP, dampingHP;
};

// Lowpass Feedback Comb Filter (LFCF)
class CombFilter {
public:
    CombFilter();
    void setSize(int samples);
    float process(float input);
    void setDamp(float val);
    void setFeedback(float val);
    void clear();

    // Make buffer accessible for size checking
    std::vector<float> buffer;
    float feedback;  // Made public for fade-out reset

private:
    int writeIndex;
    OnePole lowpass;
    float damp;
};

// Allpass filter for diffusion
class AllpassFilter {
public:
    AllpassFilter();
    void setSize(int samples);
    float process(float input);
    void setCoeff(float val);
    void clear();

    // Make buffer accessible for size checking
    std::vector<float> buffer;

private:
    int writeIndex;
    float coeff;
};

// Simple delay line
class DelayLine {
public:
    DelayLine();
    void setSize(int samples);
    void setDelay(int samples);
    float process(float input);
    void clear();

    // Make buffer accessible for size checking
    std::vector<float> buffer;

private:
    int writeIndex, readIndex, delaySamples;
};

// Main Reverb Processor
class ReverbProcessor {
public:
    ReverbProcessor();
    void prepare(double sr);
    void clear();
    void processStereo(float* left, float* right, int numSamples);

    // Get current reverb tail level (for visualization)
    float getReverbLevel() const { return reverbLevel; }

    // Reset with fade to avoid clicks
    void resetWithFade();

    // Parameter setters
    void setDecayTime(float seconds);
    void setPreDelay(float ms);
    void setDamping(float val);
    void setDiffusion(float val);
    void setReverbDiffusion(float val);
    void setRoomSize(float val);
    void setRoomVolume(float val);
    void setEarlyReflectionLevel(float val);
    void setReflectionDelay(float val);
    void setSubsequentReverbDelay(float val);
    void setSubsequentLevel(float val);
    void setEnvelopment(float val);
    void setNormalizedReflectivity(float val);
    void setTieLevel(float val);
    void setPosition(float val);
    void setDryWet(float val);

private:
    float sampleRate = 44100.0f;

    // Base delay times in milliseconds (Schroeder algorithm)
    std::vector<float> baseCombDelaysMs = { 29.7f, 37.1f, 41.1f, 43.7f, 31.3f, 34.9f, 39.5f, 44.3f };
    std::vector<float> baseAllpassDelaysMs = { 5.0f, 1.7f, 12.7f, 9.3f };
    std::vector<float> earlyTapDelaysMs = { 8.3f, 11.7f, 15.2f, 19.8f, 24.1f, 28.9f };

    std::vector<CombFilter> combsL, combsR;
    std::vector<AllpassFilter> allpassesL, allpassesR;
    DelayLine preDelayL, preDelayR;
    std::vector<std::pair<DelayLine, DelayLine>> earlyTaps;

    // Reverb parameters
    float decayTime = 2.0f, preDelayMs = 20.0f, damping = 0.5f, diffusion = 0.7f, reverbDiffusion = 0.7f;
    float roomSize = 0.75f, roomVolume = 1.0f, earlyReflectionLevel = 0.3f, reflectionDelay = 1.0f;
    float subsequentReverbDelay = 1.0f, subsequentLevel = 0.8f, envelopment = 0.8f;
    float normalizedReflectivity = 0.8f, tieLevel = 0.5f, tieLevelGain = 1.0f, position = 0.5f, dryWet = 0.5f;

    // For visualization and debugging
    float reverbLevel = 0.0f;

    // Smoothing filters for parameter changes
    juce::LinearSmoothedValue<float> decaySmoother, dampingSmoother, mixSmoother;

    // Initialize smoothers
    void initSmoothers(double sampleRate);

    int msToSamples(float ms);
    void updateAllParameters();
    void updateFeedback();
    void updateDamping();
    void updateDiffusion();
    void updatePreDelay();
    void updateTieLevel();
    void updateReflectionDelays();
    void updateSubsequentDelays();
};

// Audio Processor
class DSP256XLReverbProcessor : public juce::AudioProcessor {
public:
    DSP256XLReverbProcessor();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override;
    void releaseResources() override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String&) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    // Access to parameter tree
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

private:
    ReverbProcessor reverb;

    // Parameter state management (JUCE 8 style)
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DSP256XLReverbProcessor)
};