#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// OnePole Implementation (Enhanced)
//==============================================================================

OnePole::OnePole() : state(0.0f) {}

float OnePole::process(float input, float coeff) {
    coeff = juce::jlimit(0.0f, 0.9999f, coeff);
    state = input * (1.0f - coeff) + state * coeff;
    return state;
}

void OnePole::reset() { state = 0.0f; }
void OnePole::clear() { state = 0.0f; }

//==============================================================================
// DampingFilter Implementation
//==============================================================================

DampingFilter::DampingFilter() : lpState(0.0f), hpState(0.0f), lpCoeff(0.5f), hpCoeff(0.8f) {}

void DampingFilter::setCoeffs(float lpCoeff, float hpCoeff) {
    this->lpCoeff = juce::jlimit(0.0f, 0.999f, lpCoeff);
    this->hpCoeff = juce::jlimit(0.01f, 0.999f, hpCoeff);
}

float DampingFilter::process(float input) {
    // Multi-stage damping for frequency-dependent decay
    float stage1 = input * (1.0f - lpCoeff) + lpState * lpCoeff;
    lpState = stage1;

    // High-frequency emphasis
    float stage2 = stage1 - hpState;
    hpState = stage1 * hpCoeff + hpState * (1.0f - hpCoeff);

    return stage1 + stage2 * 0.3f;  // Mix of damped and emphasized
}

void DampingFilter::clear() {
    lpState = hpState = 0.0f;
}

//==============================================================================
// EnhancedCombFilter Implementation
//==============================================================================

EnhancedCombFilter::EnhancedCombFilter() : writeIndex(0), feedback(0.5f), dampingLP(0.5f), dampingHP(0.8f) {}

void EnhancedCombFilter::setSize(int samples) {
    if (samples <= 0) {
        DBG("ERROR: EnhancedCombFilter::setSize called with invalid size: " << samples);
        samples = 1;
    }
    buffer.resize(samples);
    clear();
    writeIndex = 0;
}

float EnhancedCombFilter::process(float input, float stereoSpread) {
    if (buffer.empty()) return input;

    if (writeIndex >= buffer.size()) {
        writeIndex = 0;
    }

    float output = buffer[writeIndex];
    float damped = damping.process(output);

    // Stereo detuning for width
    float spreadMod = 1.0f + (stereoSpread * 0.01f);
    float safeFeedback = juce::jlimit(0.0f, 0.999f, feedback * spreadMod);

    buffer[writeIndex] = input + damped * safeFeedback;
    writeIndex = (writeIndex + 1) % buffer.size();

    return output;
}

void EnhancedCombFilter::setDamping(float lpVal, float hpVal) {
    dampingLP = juce::jlimit(0.0f, 0.999f, lpVal);
    dampingHP = juce::jlimit(0.01f, 0.999f, hpVal);
    damping.setCoeffs(dampingLP, dampingHP);
}

void EnhancedCombFilter::setFeedback(float val) {
    feedback = juce::jlimit(0.0f, 0.999f, val);
}

void EnhancedCombFilter::clear() {
    std::fill(buffer.begin(), buffer.end(), 0.0f);
    damping.clear();
}

//==============================================================================
// Original CombFilter Implementation (kept for compatibility)
//==============================================================================

CombFilter::CombFilter() : writeIndex(0), damp(0.5f), feedback(0.5f) {}

void CombFilter::setSize(int samples) {
    if (samples <= 0) {
        DBG("ERROR: CombFilter::setSize called with invalid size: " << samples);
        samples = 1;
    }
    buffer.resize(samples);
    clear();
    writeIndex = 0;
}

float CombFilter::process(float input) {
    if (buffer.empty()) return input;

    if (writeIndex >= buffer.size()) {
        writeIndex = 0;
    }

    float output = buffer[writeIndex];
    float damped = lowpass.process(output, damp);

    float safeFeedback = juce::jlimit(0.0f, 0.9999f, feedback);
    buffer[writeIndex] = input + damped * safeFeedback;
    writeIndex = (writeIndex + 1) % buffer.size();
    return output;
}

void CombFilter::setDamp(float val) {
    damp = juce::jlimit(0.0f, 0.9999f, val);
}

void CombFilter::setFeedback(float val) {
    feedback = juce::jlimit(0.0f, 0.9999f, val);
}

void CombFilter::clear() {
    std::fill(buffer.begin(), buffer.end(), 0.0f);
    lowpass.clear();
}

//==============================================================================
// AllpassFilter Implementation
//==============================================================================

AllpassFilter::AllpassFilter() : writeIndex(0), coeff(0.5f) {}

void AllpassFilter::setSize(int samples) {
    if (samples <= 0) {
        DBG("ERROR: AllpassFilter::setSize called with invalid size: " << samples);
        samples = 1;
    }
    buffer.resize(samples);
    clear();
    writeIndex = 0;
}

float AllpassFilter::process(float input) {
    if (buffer.empty()) return input;

    if (writeIndex >= buffer.size()) {
        writeIndex = 0;
    }

    float bufOut = buffer[writeIndex];
    float safeCoeff = juce::jlimit(0.0f, 0.9999f, coeff);
    float output = -input + bufOut;
    buffer[writeIndex] = input + bufOut * safeCoeff;
    writeIndex = (writeIndex + 1) % buffer.size();
    return output;
}

void AllpassFilter::setCoeff(float val) {
    coeff = juce::jlimit(0.0f, 0.9999f, val);
}

void AllpassFilter::clear() {
    std::fill(buffer.begin(), buffer.end(), 0.0f);
}

//==============================================================================
// DelayLine Implementation
//==============================================================================

DelayLine::DelayLine() : writeIndex(0), readIndex(0), delaySamples(0) {}

void DelayLine::setSize(int samples) {
    if (samples <= 0) {
        DBG("ERROR: DelayLine::setSize called with invalid size: " << samples);
        samples = 1;
    }
    buffer.resize(samples);
    clear();
    writeIndex = 0;
    readIndex = 0;
}

void DelayLine::setDelay(int samples) {
    if (buffer.empty()) return;
    delaySamples = juce::jlimit(0, (int)buffer.size() - 1, samples);
    readIndex = (writeIndex - delaySamples + buffer.size()) % buffer.size();
}

float DelayLine::process(float input) {
    if (buffer.empty()) return input;

    if (writeIndex >= buffer.size()) writeIndex = 0;
    if (readIndex >= buffer.size()) readIndex = 0;

    float output = buffer[readIndex];
    buffer[writeIndex] = input;
    writeIndex = (writeIndex + 1) % buffer.size();
    readIndex = (readIndex + 1) % buffer.size();
    return output;
}

void DelayLine::clear() {
    std::fill(buffer.begin(), buffer.end(), 0.0f);
    writeIndex = 0;
    readIndex = 0;
}

//==============================================================================
// ReverbProcessor Implementation (FIXED VERSION)
//==============================================================================

ReverbProcessor::ReverbProcessor() {
    // Ensure all parameters match APVTS defaults
    decayTime = 2.0f;
    preDelayMs = 20.0f;
    damping = 0.5f;
    diffusion = 0.7f;
    reverbDiffusion = 0.7f;
    roomSize = 0.75f;
    roomVolume = 1.0f;
    earlyReflectionLevel = 0.3f;
    reflectionDelay = 1.0f;
    subsequentReverbDelay = 1.0f;
    subsequentLevel = 0.8f;
    envelopment = 0.8f;
    normalizedReflectivity = 0.8f;
    tieLevel = 0.5f;
    position = 0.5f;
    dryWet = 0.5f;
    tieLevelGain = 1.0f;
    reverbLevel = 0.0f;
}

void ReverbProcessor::prepare(double sr) {
    sampleRate = juce::jlimit(22050.0f, 192000.0f, (float)sr);

    // Initialize smoothers
    initSmoothers(sampleRate);

    // Initial filter setup
    combsL.clear();
    combsR.clear();
    allpassesL.clear();
    allpassesR.clear();

    // Create 8 comb filters for each channel
    for (int i = 0; i < 8; ++i) {
        combsL.push_back(CombFilter());
        combsR.push_back(CombFilter());
    }

    // Create 4 allpass filters for each channel
    for (int i = 0; i < 4; ++i) {
        allpassesL.push_back(AllpassFilter());
        allpassesR.push_back(AllpassFilter());
    }

    // Create early reflection taps
    earlyTaps.clear();
    for (int i = 0; i < 6; ++i) {
        earlyTaps.push_back({ DelayLine(), DelayLine() });
    }

    // Now update all parameters
    updateAllParameters();
    clear();

    DBG("ReverbProcessor prepared. SR: " << sampleRate << "Hz, "
        << combsL.size() << " combs per channel");
}

void ReverbProcessor::initSmoothers(double sampleRate) {
    // Set smoothing time constants (50ms)
    float smoothTime = 0.05f;
    decaySmoother.reset(sampleRate, smoothTime);
    dampingSmoother.reset(sampleRate, smoothTime);
    mixSmoother.reset(sampleRate, smoothTime);

    decaySmoother.setCurrentAndTargetValue(decayTime);
    dampingSmoother.setCurrentAndTargetValue(damping);
    mixSmoother.setCurrentAndTargetValue(dryWet);
}

void ReverbProcessor::clear() {
    for (auto& c : combsL) c.clear();
    for (auto& c : combsR) c.clear();
    for (auto& a : allpassesL) a.clear();
    for (auto& a : allpassesR) a.clear();
    preDelayL.clear();
    preDelayR.clear();
    for (auto& tap : earlyTaps) {
        tap.first.clear();
        tap.second.clear();
    }

    reverbLevel = 0.0f;
    DBG("All filters cleared");
}

void ReverbProcessor::resetWithFade() {
    // Simple fade-out to avoid clicks
    static constexpr int fadeSamples = 64;
    for (int i = 0; i < fadeSamples; ++i) {
        float fade = 1.0f - (float)i / fadeSamples;
        for (auto& c : combsL) c.setFeedback(c.feedback * fade);
        for (auto& c : combsR) c.setFeedback(c.feedback * fade);
    }
    clear();
    DBG("Reset with fade applied");
}

void ReverbProcessor::setDecayTime(float seconds) {
    decayTime = juce::jlimit(0.01f, 60.0f, seconds);
    decaySmoother.setTargetValue(decayTime);
    updateFeedback();
}

void ReverbProcessor::setPreDelay(float ms) {
    preDelayMs = juce::jlimit(0.0f, 500.0f, ms);
    updatePreDelay();
}

void ReverbProcessor::setDamping(float val) {
    damping = juce::jlimit(0.0f, 0.999f, val);
    dampingSmoother.setTargetValue(damping);
    updateDamping();
}

void ReverbProcessor::setDiffusion(float val) {
    diffusion = juce::jlimit(0.0f, 1.0f, val);
    updateDiffusion();
}

void ReverbProcessor::setReverbDiffusion(float val) {
    reverbDiffusion = juce::jlimit(0.0f, 1.0f, val);
    updateDiffusion();
}

void ReverbProcessor::setRoomSize(float val) {
    roomSize = juce::jlimit(0.01f, 2.0f, val);
    updateAllParameters();
}

void ReverbProcessor::setRoomVolume(float val) {
    roomVolume = juce::jlimit(0.0f, 5.0f, val);
}

void ReverbProcessor::setEarlyReflectionLevel(float val) {
    earlyReflectionLevel = juce::jlimit(0.0f, 1.0f, val);
}

void ReverbProcessor::setReflectionDelay(float val) {
    reflectionDelay = juce::jlimit(0.1f, 4.0f, val);
    updateReflectionDelays();
}

void ReverbProcessor::setSubsequentReverbDelay(float val) {
    subsequentReverbDelay = juce::jlimit(0.1f, 4.0f, val);
    updateSubsequentDelays();
}

void ReverbProcessor::setSubsequentLevel(float val) {
    subsequentLevel = juce::jlimit(0.0f, 1.0f, val);
}

void ReverbProcessor::setEnvelopment(float val) {
    envelopment = juce::jlimit(0.0f, 1.0f, val);
}

void ReverbProcessor::setNormalizedReflectivity(float val) {
    normalizedReflectivity = juce::jlimit(0.0f, 1.0f, val);
    updateFeedback();
}

void ReverbProcessor::setTieLevel(float val) {
    tieLevel = juce::jlimit(0.0f, 1.0f, val);
    updateTieLevel();
}

void ReverbProcessor::setPosition(float val) {
    position = juce::jlimit(0.0f, 1.0f, val);
}

void ReverbProcessor::setDryWet(float val) {
    dryWet = juce::jlimit(0.0f, 1.0f, val);
    mixSmoother.setTargetValue(dryWet);
}

// FIXED AUDIO PROCESSING METHOD - MAIN FIX
void ReverbProcessor::processStereo(float* left, float* right, int numSamples) {
    // Sanity check inputs
    if (left == nullptr || right == nullptr || numSamples <= 0) {
        DBG("ERROR: Invalid inputs to processStereo");
        return;
    }

    // Ensure we have valid state
    if (combsL.empty() || combsR.empty()) {
        DBG("ERROR: Filters not initialized in processStereo!");
        return;
    }

    // Get smoothed parameter values
    float currentDecay = decaySmoother.getNextValue();
    float currentDamping = dampingSmoother.getNextValue();
    float currentMix = mixSmoother.getNextValue();

    for (int i = 0; i < numSamples; ++i) {
        float dryL = left[i];
        float dryR = right[i];

        // Apply input gain/volume with soft limiting
        float gain = roomVolume;
        dryL *= juce::jlimit(0.0f, 2.0f, gain);
        dryR *= juce::jlimit(0.0f, 2.0f, gain);

        // Pre-delay (preserves stereo)
        float preL = preDelayL.process(dryL);
        float preR = preDelayR.process(dryR);

        // Early reflections - maintain stereo image
        float earlyL = 0.0f, earlyR = 0.0f;
        for (size_t t = 0; t < earlyTaps.size(); ++t) {
            // Progressive panning across taps for natural stereo
            float pan = (float)t / earlyTaps.size();
            earlyL += earlyTaps[t].first.process(preL) * (1.0f - pan * 0.7f);
            earlyR += earlyTaps[t].second.process(preR) * (0.3f + pan * 0.7f);
        }
        earlyL = earlyL * earlyReflectionLevel / earlyTaps.size();
        earlyR = earlyR * earlyReflectionLevel / earlyTaps.size();

        // CRITICAL FIX: Process LEFT and RIGHT channels SEPARATELY
        // with proper cross-coupling for natural stereo reverb

        // Process left channel through left combs with cross-feed from right
        float combSumL = 0.0f;
        for (size_t c = 0; c < combsL.size(); ++c) {
            // Each comb gets a unique mix of L/R for natural stereo spread
            float leftWeight = 0.7f + 0.3f * std::sin((float)c * 0.5f);
            float rightWeight = 0.3f * std::cos((float)c * 0.5f);

            float combInput = (preL * leftWeight + preR * rightWeight * position);

            // Add slight detuning between combs for richer sound
            float detune = 1.0f + (0.0005f * c);
            combSumL += combsL[c].process(combInput * detune);
        }
        combSumL /= combsL.size();

        // Process right channel through right combs with cross-feed from left
        float combSumR = 0.0f;
        for (size_t c = 0; c < combsR.size(); ++c) {
            float rightWeight = 0.7f + 0.3f * std::cos((float)c * 0.5f);
            float leftWeight = 0.3f * std::sin((float)c * 0.5f);

            float combInput = (preR * rightWeight + preL * leftWeight * (1.0f - position));

            float detune = 1.0f - (0.0005f * c);
            combSumR += combsR[c].process(combInput * detune);
        }
        combSumR /= combsR.size();

        // Apply allpass diffusion (series) for smoother tail
        float diffusedL = combSumL;
        float diffusedR = combSumR;
        for (size_t a = 0; a < allpassesL.size(); ++a) {
            diffusedL = allpassesL[a].process(diffusedL);
            diffusedR = allpassesR[a].process(diffusedR);
        }

        // Apply subsequent/tail level with HF emphasis
        float tailLevel = subsequentLevel * tieLevelGain;
        diffusedL *= tailLevel;
        diffusedR *= tailLevel;

        // M/S processing with envelopment control for width
        float mid = (diffusedL + diffusedR) * 0.707f;
        float side = (diffusedL - diffusedR) * 0.707f;

        float wetL = mid + side * envelopment;
        float wetR = mid - side * envelopment;

        // Combine early reflections + late reverb with energy conservation
        float earlyMix = 0.3f;
        float lateMix = 0.7f;
        wetL = (earlyL * earlyMix + wetL * lateMix) * normalizedReflectivity;
        wetR = (earlyR * earlyMix + wetR * lateMix) * normalizedReflectivity;

        // Apply frequency contour (tieLevel affects high frequencies)
        float hfBoost = tieLevel * 2.0f;
        wetL = wetL * (1.0f + hfBoost * 0.5f);
        wetR = wetR * (1.0f + hfBoost * 0.5f);

        // Final dry/wet mix with smooth transition
        float smoothMix = currentMix;
        if (i == 0) {
            smoothMix = mixSmoother.getCurrentValue();
        }

        left[i] = dryL * (1.0f - smoothMix) + wetL * smoothMix;
        right[i] = dryR * (1.0f - smoothMix) + wetR * smoothMix;

        // Update reverb level for visualization
        reverbLevel = 0.995f * reverbLevel + 0.005f * std::sqrt(wetL * wetL + wetR * wetR);

        // Protect against clipping with soft limiting
        left[i] = juce::jlimit(-1.0f, 1.0f, left[i] * 0.95f);
        right[i] = juce::jlimit(-1.0f, 1.0f, right[i] * 0.95f);

        // Update smoothers
        if (i % 8 == 0) {  // Update less frequently for performance
            currentDecay = decaySmoother.getNextValue();
            currentDamping = dampingSmoother.getNextValue();
            currentMix = mixSmoother.getNextValue();
        }
    }
}

int ReverbProcessor::msToSamples(float ms) {
    if (ms < 0.0f) ms = 0.0f;
    if (sampleRate <= 0.0f) {
        DBG("ERROR: Invalid sample rate in msToSamples: " << sampleRate);
        sampleRate = 44100.0f;
    }
    return static_cast<int>(sampleRate * ms / 1000.0f);
}

void ReverbProcessor::updateAllParameters() {
    if (sampleRate <= 0.0f) {
        DBG("ERROR: updateAllParameters called before sample rate was set!");
        return;
    }

    // Track changes to avoid unnecessary updates
    static float lastRoomSize = roomSize;
    static float lastRefDelay = reflectionDelay;
    static float lastSubDelay = subsequentReverbDelay;

    bool sizeChanged = std::abs(roomSize - lastRoomSize) > 0.001f;
    bool refChanged = std::abs(reflectionDelay - lastRefDelay) > 0.001f;
    bool subChanged = std::abs(subsequentReverbDelay - lastSubDelay) > 0.001f;

    // Only update if parameters changed significantly
    if (!sizeChanged && !refChanged && !subChanged &&
        !combsL.empty() && !combsR.empty()) {
        // Update other parameters without recreating filters
        updateFeedback();
        updateDamping();
        updateDiffusion();
        updatePreDelay();
        updateTieLevel();
        return;
    }

    float sizeScalar = juce::jlimit(0.01f, 2.0f, roomSize);

    // Update comb filters
    for (size_t i = 0; i < combsL.size() && i < baseCombDelaysMs.size(); ++i) {
        float delayMs = baseCombDelaysMs[i] * sizeScalar * subsequentReverbDelay;
        int delaySamples = msToSamples(delayMs);

        if (delaySamples < 1) delaySamples = 1;

        // Only resize if needed
        if (combsL[i].buffer.size() != delaySamples) {
            combsL[i].setSize(delaySamples);
        }
        if (combsR[i].buffer.size() != msToSamples(delayMs * 1.02f)) {
            combsR[i].setSize(msToSamples(delayMs * 1.02f));
        }
    }

    // Update allpass filters
    for (size_t i = 0; i < allpassesL.size() && i < baseAllpassDelaysMs.size(); ++i) {
        float delayMs = baseAllpassDelaysMs[i] * sizeScalar;
        int delaySamples = msToSamples(delayMs);

        if (delaySamples < 1) delaySamples = 1;

        if (allpassesL[i].buffer.size() != delaySamples) {
            allpassesL[i].setSize(delaySamples);
        }
        if (allpassesR[i].buffer.size() != msToSamples(delayMs * 1.02f)) {
            allpassesR[i].setSize(msToSamples(delayMs * 1.02f));
        }
    }

    // Update early reflection taps
    for (size_t t = 0; t < earlyTaps.size() && t < earlyTapDelaysMs.size(); ++t) {
        float delayMs = earlyTapDelaysMs[t] * sizeScalar * reflectionDelay;

        int delaySamplesL = msToSamples(delayMs);
        int delaySamplesR = msToSamples(delayMs * 1.03f);

        // Set size if needed, then delay
        if (earlyTaps[t].first.buffer.size() < delaySamplesL) {
            earlyTaps[t].first.setSize(delaySamplesL * 2);  // Extra headroom
        }
        earlyTaps[t].first.setDelay(delaySamplesL);

        if (earlyTaps[t].second.buffer.size() < delaySamplesR) {
            earlyTaps[t].second.setSize(delaySamplesR * 2);
        }
        earlyTaps[t].second.setDelay(delaySamplesR);
    }

    // Update all other parameters
    updateFeedback();
    updateDamping();
    updateDiffusion();
    updatePreDelay();
    updateTieLevel();

    // Store current values
    lastRoomSize = roomSize;
    lastRefDelay = reflectionDelay;
    lastSubDelay = subsequentReverbDelay;

    DBG("All parameters updated. Room size: " << roomSize
        << ", Comb count: " << combsL.size());
}

void ReverbProcessor::updateFeedback() {
    if (combsL.empty() || combsR.empty() || decayTime <= 0.0f || sampleRate <= 0.0f) {
        DBG("ERROR: updateFeedback called with invalid state");
        return;
    }

    // Calculate feedback for RT60 decay time
    float avgCombDelayMs = baseCombDelaysMs[0] * roomSize * subsequentReverbDelay;
    float avgCombDelaySec = avgCombDelayMs / 1000.0f;

    if (decayTime < 0.01f) decayTime = 0.01f;

    // Formula: g = 10^(-3 * delay / decayTime)
    float exponent = -3.0f * avgCombDelaySec / decayTime;
    float baseFeedback = std::pow(10.0f, exponent);

    // Apply reflectivity
    baseFeedback *= normalizedReflectivity;

    // Safety limit with margin
    baseFeedback = juce::jlimit(0.0f, 0.998f, baseFeedback);

    // Apply with slight variations between combs for richer sound
    for (size_t i = 0; i < combsL.size(); ++i) {
        float variation = 1.0f + (0.015f * (i % 4));
        combsL[i].setFeedback(baseFeedback * variation);
        combsR[i].setFeedback(baseFeedback * (1.0f / variation));
    }

    DBG("Feedback updated: " << baseFeedback << " for decayTime: " << decayTime);
}

void ReverbProcessor::updateDamping() {
    // Separate damping for low and high frequencies
    float lpDamp = damping * 0.9f;
    float hpDamp = 0.1f + damping * 0.4f;

    for (auto& c : combsL) c.setDamp(lpDamp);
    for (auto& c : combsR) c.setDamp(lpDamp);

    // Note: For enhanced comb filters, you'd use setDamping(lpDamp, hpDamp)
    DBG("Damping updated: LP=" << lpDamp << ", HP=" << hpDamp);
}

void ReverbProcessor::updateDiffusion() {
    float earlyCoeff = diffusion * 0.6f;
    float tailCoeff = reverbDiffusion * 0.6f;

    earlyCoeff = juce::jlimit(0.01f, 0.999f, earlyCoeff);
    tailCoeff = juce::jlimit(0.01f, 0.999f, tailCoeff);

    for (size_t i = 0; i < allpassesL.size(); ++i) {
        float coeff = (i < 2) ? earlyCoeff : tailCoeff;
        allpassesL[i].setCoeff(coeff);
        allpassesR[i].setCoeff(coeff);
    }

    DBG("Diffusion updated: early=" << earlyCoeff << ", tail=" << tailCoeff);
}

void ReverbProcessor::updatePreDelay() {
    int maxPreDelay = msToSamples(500.0f);
    if (maxPreDelay < 1) maxPreDelay = 1;

    // Only resize if needed
    if (preDelayL.buffer.size() < maxPreDelay) {
        preDelayL.setSize(maxPreDelay);
        preDelayR.setSize(maxPreDelay);
    }

    int delaySamples = msToSamples(preDelayMs);
    preDelayL.setDelay(delaySamples);
    preDelayR.setDelay(delaySamples);

    DBG("Pre-delay updated: " << preDelayMs << "ms (" << delaySamples << " samples)");
}

void ReverbProcessor::updateTieLevel() {
    // Calculate HF level gain
    tieLevelGain = 0.5f + tieLevel * 1.5f;
    tieLevelGain = juce::jlimit(0.0f, 3.0f, tieLevelGain);

    DBG("Tie level updated: " << tieLevel << " -> gain: " << tieLevelGain);
}

void ReverbProcessor::updateReflectionDelays() {
    for (size_t t = 0; t < earlyTaps.size() && t < earlyTapDelaysMs.size(); ++t) {
        float delayMs = earlyTapDelaysMs[t] * roomSize * reflectionDelay;
        int delaySamplesL = msToSamples(delayMs);
        int delaySamplesR = msToSamples(delayMs * 1.03f);

        earlyTaps[t].first.setDelay(delaySamplesL);
        earlyTaps[t].second.setDelay(delaySamplesR);
    }
}

void ReverbProcessor::updateSubsequentDelays() {
    for (size_t i = 0; i < combsL.size() && i < baseCombDelaysMs.size(); ++i) {
        float delayMs = baseCombDelaysMs[i] * roomSize * subsequentReverbDelay;
        int delaySamplesL = msToSamples(delayMs);
        int delaySamplesR = msToSamples(delayMs * 1.02f);

        // Only resize if needed
        if (combsL[i].buffer.size() != delaySamplesL) {
            combsL[i].setSize(delaySamplesL);
        }
        if (combsR[i].buffer.size() != delaySamplesR) {
            combsR[i].setSize(delaySamplesR);
        }
    }
    updateFeedback();
}

//==============================================================================
// DSP256XLReverbProcessor Implementation
//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout DSP256XLReverbProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Core Reverb Parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("decay", 1), "Decay Time",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f, 0.5f), 2.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 2) + "s"; }));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("predelay", 1), "Pre Delay",
        juce::NormalisableRange<float>(0.0f, 200.0f, 1.0f), 20.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 0) + "ms"; }));

    // Damping & Diffusion
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("damping", 1), "Damping",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("diffusion", 1), "Diffusion",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.7f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("revdiff", 1), "Reverb Diffusion",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.7f));

    // Room Parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("size", 1), "Room Size",
        juce::NormalisableRange<float>(0.1f, 2.0f, 0.01f, 0.7f), 0.75f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value * 100, 0) + "%"; }));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("volume", 1), "Room Volume",
        juce::NormalisableRange<float>(0.0f, 3.0f, 0.01f), 1.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 2) + "x"; }));

    // Early Reflections
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("early", 1), "Early Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value * 100, 0) + "%"; }));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("refdelay", 1), "Reflection Delay",
        juce::NormalisableRange<float>(0.5f, 2.0f, 0.01f), 1.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 2) + "x"; }));

    // Subsequent/Tail Parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("subdelay", 1), "Subsequent Delay",
        juce::NormalisableRange<float>(0.5f, 2.0f, 0.01f), 1.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 2) + "x"; }));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sublevel", 1), "Subsequent Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value * 100, 0) + "%"; }));

    // Spatial Parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("envelop", 1), "Envelopment",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value * 100, 0) + "%"; }));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("position", 1), "Position",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            if (value < 0.33f) return "Left";
            if (value < 0.66f) return "Center";
            return "Right";
        }));

    // Surface & Tone
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("reflect", 1), "Reflectivity",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value * 100, 0) + "%"; }));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("tielevel", 1), "HF Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value * 100, 0) + "%"; }));

    // Mix
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("mix", 1), "Dry/Wet",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value * 100, 0) + "%"; }));

    return layout;
}

DSP256XLReverbProcessor::DSP256XLReverbProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Ensure reverb processor matches APVTS defaults
    reverb.setDecayTime(2.0f);
    reverb.setPreDelay(20.0f);
    reverb.setDamping(0.5f);
    reverb.setDiffusion(0.7f);
    reverb.setReverbDiffusion(0.7f);
    reverb.setRoomSize(0.75f);
    reverb.setRoomVolume(1.0f);
    reverb.setEarlyReflectionLevel(0.3f);
    reverb.setReflectionDelay(1.0f);
    reverb.setSubsequentReverbDelay(1.0f);
    reverb.setSubsequentLevel(0.8f);
    reverb.setEnvelopment(0.8f);
    reverb.setNormalizedReflectivity(0.8f);
    reverb.setTieLevel(0.5f);
    reverb.setPosition(0.5f);
    reverb.setDryWet(0.5f);

    DBG("DSP256XLReverbProcessor constructed with " <<
        apvts.getParameter("decay")->getValue() << " decay");
}

void DSP256XLReverbProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    if (sampleRate <= 0.0) {
        DBG("ERROR: prepareToPlay called with invalid sample rate: " << sampleRate);
        sampleRate = 44100.0;
    }

    reverb.prepare(sampleRate);

    DBG("Prepared to play at " << sampleRate << "Hz, block size: " << samplesPerBlock);
}

void DSP256XLReverbProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;

    if (buffer.getNumChannels() < 2) {
        DBG("WARNING: processBlock called with < 2 channels");
        return;
    }

    // Optional: Check for input level
    float inputLevel = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        const float* channelData = buffer.getReadPointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            inputLevel = std::max(inputLevel, std::abs(channelData[i]));
        }
    }

    // Get all parameters from APVTS
    reverb.setDecayTime(apvts.getRawParameterValue("decay")->load());
    reverb.setPreDelay(apvts.getRawParameterValue("predelay")->load());
    reverb.setDamping(apvts.getRawParameterValue("damping")->load());
    reverb.setDiffusion(apvts.getRawParameterValue("diffusion")->load());
    reverb.setReverbDiffusion(apvts.getRawParameterValue("revdiff")->load());
    reverb.setRoomSize(apvts.getRawParameterValue("size")->load());
    reverb.setRoomVolume(apvts.getRawParameterValue("volume")->load());
    reverb.setEarlyReflectionLevel(apvts.getRawParameterValue("early")->load());
    reverb.setReflectionDelay(apvts.getRawParameterValue("refdelay")->load());
    reverb.setSubsequentReverbDelay(apvts.getRawParameterValue("subdelay")->load());
    reverb.setSubsequentLevel(apvts.getRawParameterValue("sublevel")->load());
    reverb.setEnvelopment(apvts.getRawParameterValue("envelop")->load());
    reverb.setPosition(apvts.getRawParameterValue("position")->load());
    reverb.setNormalizedReflectivity(apvts.getRawParameterValue("reflect")->load());
    reverb.setTieLevel(apvts.getRawParameterValue("tielevel")->load());
    reverb.setDryWet(apvts.getRawParameterValue("mix")->load());

    // Process stereo audio
    float* left = buffer.getWritePointer(0);
    float* right = buffer.getWritePointer(1);

    reverb.processStereo(left, right, buffer.getNumSamples());

    // Optional: Check output level
    float outputLevel = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        const float* channelData = buffer.getReadPointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            outputLevel = std::max(outputLevel, std::abs(channelData[i]));
        }
    }

    static int debugCounter = 0;
    if (debugCounter++ % 100 == 0) {
        DBG("Processing: input=" << inputLevel << ", output=" << outputLevel
            << ", reverbLevel=" << reverb.getReverbLevel());
    }
}

void DSP256XLReverbProcessor::releaseResources() {
    reverb.clear();
    DBG("Resources released");
}

const juce::String DSP256XLReverbProcessor::getName() const {
    return "DSP-256XL Reverb";
}

bool DSP256XLReverbProcessor::acceptsMidi() const { return false; }
bool DSP256XLReverbProcessor::producesMidi() const { return false; }
double DSP256XLReverbProcessor::getTailLengthSeconds() const {
    return apvts.getRawParameterValue("decay")->load() * 2.0;
}
int DSP256XLReverbProcessor::getNumPrograms() { return 1; }
int DSP256XLReverbProcessor::getCurrentProgram() { return 0; }
void DSP256XLReverbProcessor::setCurrentProgram(int) {}
const juce::String DSP256XLReverbProcessor::getProgramName(int) { return "Default"; }
void DSP256XLReverbProcessor::changeProgramName(int, const juce::String&) {}

void DSP256XLReverbProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
    DBG("State saved");
}

void DSP256XLReverbProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr && xmlState->hasTagName(apvts.state.getType())) {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        DBG("State restored");
    }
}

juce::AudioProcessorEditor* DSP256XLReverbProcessor::createEditor() {
    return new DSP256XLReverbEditor(*this);
}

bool DSP256XLReverbProcessor::hasEditor() const {
    return true;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new DSP256XLReverbProcessor();
}