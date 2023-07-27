#pragma once

#include <JuceHeader.h>

#include "../../Common/BaseAudioProcessor.h"
#include "../../Common/PresetBar.h"

#include "DSP/ClippingFunctions.h"
#include "DSP/Delay.h"
#include "DSP/DiodeWDF.h"
#include "DSP/WidthProcessor.h"
#include "GUI/InterfaceDefines.h"
#include "Panels/SpectrogramPanel/FFTProcessor.h"
#include "Utility/AudioHelpers.h"


constexpr int MAX_NUM_BANDS = 4;

class HayesDistortionAudioProcessor : public BaseAudioProcessor
{
public:
    HayesDistortionAudioProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed (juce::AudioBuffer<float>& buffer,
                               juce::MidiBuffer& midiMessages) override;
    juce::AudioProcessorEditor* createEditor() override;

    void updateFilter();

    bool isSilent (juce::AudioBuffer<float> buffer);

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() override;

    void setHistoryArray (int bandIndex);
    juce::Array<float> getHistoryArrayL();
    juce::Array<float> getHistoryArrayR();

    float* getFFTData();
    int getNumBins();
    int getFFTSize();
    bool isFFTBlockReady();
    void pushDataToFFT();
    void processFFT (float* tempFFTData);

    void setSavedWidth (const int width);
    void setSavedHeight (const int height);
    int getSavedWidth() const;
    int getSavedHeight() const;

    bool getBypassedState();

    void setLineNum();

    float getInputMeterRMSLevel (int channel, juce::String bandName);
    float getOutputMeterRMSLevel (int channel, juce::String bandName);

    float getReductionPrecent (juce::String safeId);
    void setReductionPrecent (juce::String safeId, float reductionPrecent);
    float getSampleMaxValue (juce::String safeId);
    void setSampleMaxValue (juce::String safeId, float sampleMaxValue);

    float safeMode (float drive, juce::AudioBuffer<float>& buffer, juce::String safeID);

private:
    int lineNum = 0;

    juce::Array<float> historyArrayL;
    juce::Array<float> historyArrayR;
    int historyLength = 400;

    SpectrumProcessor spectrum_processor;

    juce::AudioBuffer<float> mDryBuffer;
    juce::AudioBuffer<float> mWetBuffer;

    MonoChain leftChain;
    MonoChain rightChain;

    void updateLowCutFilters (const ChainSettings& chainSettings);
    void updateHighCutFilters (const ChainSettings& chainSettings);
    void updatePeakFilter (const ChainSettings& chainSettings);

    float previousOutputs[MAX_NUM_BANDS] = { 0.0f };
    float previousOutput = 0.0f;
    float previousDrives[MAX_NUM_BANDS] = { 0.0f };
    float previousMixes[MAX_NUM_BANDS] = { 0.0f };
    float previousMix = 0.0f;
    float previousLowcutFreq = 0.0f;
    float previousHighcutFreq = 0.0f;
    float previousPeakFreq = 0.0f;

    float newDrives[MAX_NUM_BANDS] = { 1.0f };
    float newDrive1 = 1.0f;
    float newDrive2 = 1.0f;
    float newDrive3 = 1.0f;
    float newDrive4 = 1.0f;

    juce::SmoothedValue<float> driveSmoothers[MAX_NUM_BANDS];
    juce::SmoothedValue<float> outputSmoothers[MAX_NUM_BANDS];
    juce::SmoothedValue<float> outputSmootherGlobal;
    juce::SmoothedValue<float> recSmoothers[MAX_NUM_BANDS];
    juce::SmoothedValue<float> biasSmoothers[MAX_NUM_BANDS];
    juce::SmoothedValue<float> mixSmoothers[MAX_NUM_BANDS];
    juce::SmoothedValue<float> mixSmootherGlobal;
    juce::SmoothedValue<float> lowcutFreqSmoother;
    juce::SmoothedValue<float> highcutFreqSmoother;
    juce::SmoothedValue<float> peakFreqSmoother;
    juce::SmoothedValue<float> centralSmoother;
    juce::SmoothedValue<float> normalSmoother;

    // DSP Processors
    WidthProcessor widthProcessors[MAX_NUM_BANDS];

    using GainProcessor = juce::dsp::Gain<float>;
    using BiasProcessor = juce::dsp::Bias<float>;
    using DriveProcessor = juce::dsp::WaveShaper<float>;
    using DCFilter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    using CompressorProcessor = juce::dsp::Compressor<float>;
    using LimiterProcessor = juce::dsp::Limiter<float>;
    using DryWetMixer = juce::dsp::DryWetMixer<float>;

    CompressorProcessor compressorProcessors[MAX_NUM_BANDS];

    DCFilter dcFilters[MAX_NUM_BANDS];

    juce::dsp::ProcessorChain<GainProcessor, BiasProcessor, DriveProcessor, juce::dsp::WaveShaper<float, std::function<float(float)>>, BiasProcessor> overdrives[MAX_NUM_BANDS];

    GainProcessor gainProcessors[MAX_NUM_BANDS];
    GainProcessor gainProcessorGlobal;
    
    LimiterProcessor limiterProcessorGlobal;

    DryWetMixer dryWetMixer1 { 100 };
    DryWetMixer dryWetMixer2 { 100 };
    DryWetMixer dryWetMixer3 { 100 };
    DryWetMixer dryWetMixer4 { 100 };
    DryWetMixer dryWetMixerGlobal { 100 };

    int oversampleFactor = 1;

    // oversampling delay, set to dry buffer
    Delay mDelay { 0 };

    // mode 8 diode================
    juce::Array<float> inputTemp;
    float VdiodeL;
    float VdiodeR;
    float RiL;
    float RiR;
    VoltageSource VinL;
    VoltageSource VinR;
    Resistor R1L;
    Resistor R1R;
    Capacitor C1L;
    Capacitor C1R;
    Serie RCL;
    Serie RCR;
    Serie rootL;
    Serie rootR;
    // mode 9 diode=================

    // dsp::AudioBlock<float> blockOutput;

    // multiband dsp
    juce::dsp::LinkwitzRileyFilter<float> lowpasses[MAX_NUM_BANDS - 1];
    juce::dsp::LinkwitzRileyFilter<float> highpasses[MAX_NUM_BANDS - 1];

    juce::AudioBuffer<float> mBuffers[MAX_NUM_BANDS];

    bool multibandEnables[MAX_NUM_BANDS] = { true };

    bool multibandSolos[MAX_NUM_BANDS] = { true };

    bool shouldSetBlackMask (int index);
    bool getSoloStateFromIndex (int index);

    void processOneBand (juce::AudioBuffer<float>& bandBuffer, juce::dsp::ProcessContextReplacing<float> context, juce::String modeID, juce::String driveID, juce::String safeID, juce::String extremeID, juce::String biasID, juce::String recID, juce::dsp::ProcessorChain<GainProcessor, BiasProcessor, DriveProcessor, juce::dsp::WaveShaper<float, std::function<float (float)>>, BiasProcessor>& overdrive, juce::String outputID, GainProcessor& gainProcessor, juce::String threshID, juce::String ratioID, CompressorProcessor& compressorProcessor, int totalNumInputChannels, juce::SmoothedValue<float>& recSmoother, juce::SmoothedValue<float>& outputSmoother, juce::String mixID, juce::dsp::DryWetMixer<float>& dryWetMixer, juce::String widthID, WidthProcessor widthProcessor, DCFilter& dcFilter, juce::String widthBypassID, juce::String compBypassID);

    void processDistortion (juce::AudioBuffer<float>& bandBuffer, juce::String modeID, juce::String driveID, juce::String safeID, juce::String extremeID, juce::String biasID, juce::String recID, juce::dsp::ProcessorChain<GainProcessor, BiasProcessor, DriveProcessor, juce::dsp::WaveShaper<float, std::function<float (float)>>, BiasProcessor>& overdrive, DCFilter& dcFilter);

    void processLimiter (juce::dsp::ProcessContextReplacing<float> context, juce::String limiterThreshID, juce::String limiterReleaseID, LimiterProcessor& limiterProcessor);
    void processGain (juce::dsp::ProcessContextReplacing<float> context, juce::String outputID, GainProcessor& gainProcessor);

    void processCompressor (juce::dsp::ProcessContextReplacing<float> context, juce::String threshID, juce::String ratioID, CompressorProcessor& compressor);

    void normalize (juce::String modeID, juce::AudioBuffer<float>& buffer, int totalNumInputChannels, juce::SmoothedValue<float>& recSmoother, juce::SmoothedValue<float>& outputSmoother);

    // void compressorProcessor(float ratio, float thresh, juce::dsp::Compressor<float> compressorProcessor, juce::dsp::ProcessContextReplacing<float> &context);

    void mixDryWet(juce::AudioBuffer<float>& dryBuffer, juce::AudioBuffer<float>& wetBuffer, juce::String mixID, juce::dsp::DryWetMixer<float>& dryWetMixer, float latency);

    float mLatency { 0.0f };

    // Save size
    int editorWidth = static_cast<int>(INIT_WIDTH);
    int editorHeight = static_cast<int>(INIT_HEIGHT);

    // bypass state
    bool isBypassed = false;

    // VU meters
    void setLeftRightMeterRMSValues(juce::AudioBuffer<float> buffer, float& leftOutValue, float& rightOutValue);
    float mInputLeftSmoothedGlobal   { 0 };
    float mInputRightSmoothedGlobal  { 0 };
    float mOutputLeftSmoothedGlobal  { 0 };
    float mOutputRightSmoothedGlobal { 0 };

    float mInputLeftSmoothedBands[MAX_NUM_BANDS]   { 0 };
    float mInputRightSmoothedBands[MAX_NUM_BANDS]  { 0 };
    float mOutputLeftSmoothedBands[MAX_NUM_BANDS]  { 0 };
    float mOutputRightSmoothedBands[MAX_NUM_BANDS] { 0 };

    // Drive lookandfeel
    float mReductionPrecents[MAX_NUM_BANDS] { 1 };
    float mSampleMaxValues[MAX_NUM_BANDS]   { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HayesDistortionAudioProcessor)
};
