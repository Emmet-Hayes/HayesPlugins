#include "HayesDistortionAudioProcessor.h"
#include "HayesDistortionAudioProcessorEditor.h"


HayesDistortionAudioProcessor::HayesDistortionAudioProcessor()
:   BaseAudioProcessor { createParameterLayout() }
,   VinL               { 500.f, 0.f } // VinL(0.f, 500.f)
,   VinR               { 500.f, 0.f }
,   R1L                { 80.0f }
,   R1R                { 80.0f }
,   C1L                { 3.5e-5, getSampleRate() }
,   C1R                { 3.5e-5, getSampleRate() }
,   RCL                { &R1L, &C1L }
,   RCR                { &R1R, &C1R }
,   rootL              { &VinL, &RCL }
,   rootR              { &VinR, &RCR }
{
}

void HayesDistortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    auto channels = static_cast<juce::uint32> (juce::jmin (getMainBusNumInputChannels(), getMainBusNumOutputChannels()));
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), channels };

    // fix the artifacts (also called zipper noise)
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        previousOutputs[i] = (float)*apvts.getRawParameterValue("output" + std::to_string(i + 1));
        previousOutputs[i] = juce::Decibels::decibelsToGain(previousOutputs[0]);
    }
    previousOutput = juce::Decibels::decibelsToGain ((float) *apvts.getRawParameterValue (OUTPUT_ID));

    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        previousDrives[i] = *apvts.getRawParameterValue("drive" + std::to_string(i + 1));
        previousMixes[i] = *apvts.getRawParameterValue("mix" + std::to_string(i + 1));
    }
    previousMix = (float) *apvts.getRawParameterValue (MIX_ID);

    previousLowcutFreq = (float) *apvts.getRawParameterValue (LOWCUT_FREQ_ID);
    previousHighcutFreq = (float) *apvts.getRawParameterValue (HIGHCUT_FREQ_ID);
    previousPeakFreq = (float) *apvts.getRawParameterValue (PEAK_FREQ_ID);

    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        driveSmoothers[i].reset(sampleRate, 0.05); //0.05 second is rampLength, which means increasing to targetvalue needs 0.05s.
        driveSmoothers[i].setCurrentAndTargetValue(previousDrives[0]);
        outputSmoothers[i].reset(sampleRate, 0.05);
        outputSmoothers[i].setCurrentAndTargetValue(previousOutputs[0]);
    }

    outputSmootherGlobal.reset (sampleRate, 0.05);
    outputSmootherGlobal.setCurrentAndTargetValue (previousOutput);

    lowcutFreqSmoother.reset (sampleRate, 0.001);
    lowcutFreqSmoother.setCurrentAndTargetValue (previousLowcutFreq);
    highcutFreqSmoother.reset (sampleRate, 0.001);
    highcutFreqSmoother.setCurrentAndTargetValue (previousHighcutFreq);
    peakFreqSmoother.reset (sampleRate, 0.001);
    peakFreqSmoother.setCurrentAndTargetValue (previousPeakFreq);


    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        recSmoothers[i].reset(sampleRate, 0.05);
        recSmoothers[i].setCurrentAndTargetValue(*apvts.getRawParameterValue("rec" + std::to_string(i + 1)));
        biasSmoothers[i].reset(sampleRate, 0.001);
        biasSmoothers[i].setCurrentAndTargetValue(*apvts.getRawParameterValue("bias" + std::to_string(i + 1)));
        mixSmoothers[i].reset(sampleRate, 0.05);
        mixSmoothers[i].setCurrentAndTargetValue(previousMixes[i]);
    }

    mixSmootherGlobal.reset (sampleRate, 0.05);
    mixSmootherGlobal.setCurrentAndTargetValue (previousMix);

    centralSmoother.reset (sampleRate, 0.1);
    centralSmoother.setCurrentAndTargetValue (0);

    normalSmoother.reset (sampleRate, 0.5);
    normalSmoother.setCurrentAndTargetValue (1);

    // historyArray init
    for (int i = 0; i < samplesPerBlock; i++)
    {
        historyArrayL.add (0);
        historyArrayR.add (0);
    }

    // dry wet buffer init
    mDryBuffer.setSize (getTotalNumInputChannels(), samplesPerBlock);
    mDryBuffer.clear();

    mWetBuffer.setSize (getTotalNumInputChannels(), samplesPerBlock);
    mWetBuffer.clear();

    mDelay.reset (0);

    // filter init
    updateFilter();
    leftChain.prepare (spec);
    rightChain.prepare (spec);
    // mode diode================
    inputTemp.clear();
    VdiodeL = 0.0f;
    VdiodeR = 0.0f;
    RiL = 1;
    RiR = 1;

    // multiband filters
    for (int i = 0; i < MAX_NUM_BANDS - 1; ++i)
    {
        mBuffers[i].setSize(getTotalNumOutputChannels(), samplesPerBlock);
        mBuffers[i].clear();
    }

    for (int i = 0; i < MAX_NUM_BANDS - 1; ++i)
    {
        lowpasses[i].setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        lowpasses[i].prepare(spec);
        highpasses[i].setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        highpasses[i].prepare(spec);
    }

    // limiter
    limiterProcessorGlobal.prepare(spec);
    
    // gain
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
        gainProcessors[i].prepare (spec);

    gainProcessorGlobal.prepare (spec);

    // compressors, dc filters and overdrives
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        compressorProcessors[i].prepare(spec);
        compressorProcessors[i].setAttack(80.0f);
        compressorProcessors[i].setRelease(200.0f);
        dcFilters[i].state = juce::dsp::IIR::Coefficients<float>::makeHighPass(spec.sampleRate, 20.0);
        overdrives[i].prepare(spec);
    }

    // dry wet
    dryWetMixerGlobal.prepare (spec);
    dryWetMixer1.prepare (spec);
    dryWetMixer2.prepare (spec);
    dryWetMixer3.prepare (spec);
    dryWetMixer4.prepare (spec);
    reset();
}

void HayesDistortionAudioProcessor::reset()
{
    for (int i = 0; i < MAX_NUM_BANDS - 1; ++i)
    {
        lowpasses[i].reset();
        highpasses[i].reset();
        compressorProcessors[i].reset();
    }
    compressorProcessors[MAX_NUM_BANDS - 1].reset();
    dryWetMixerGlobal.reset();
    dryWetMixer1.reset();
    dryWetMixer2.reset();
    dryWetMixer3.reset();
    dryWetMixer4.reset();
    limiterProcessorGlobal.reset();
}

void HayesDistortionAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    reset();
}

void HayesDistortionAudioProcessor::processBlockBypassed (juce::AudioBuffer<float>& /*buffer*/,
                                               juce::MidiBuffer& /*midiMessages*/)
{
    // set bypass to true
    isBypassed = true;
}

void HayesDistortionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    // set bypass to false
    if (isBypassed)
    {
        //        buffer.clear();
        isBypassed = false;
    }

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // VU input meter
    setLeftRightMeterRMSValues (buffer, mInputLeftSmoothedGlobal, mInputRightSmoothedGlobal);

    juce::dsp::AudioBlock<float> block (buffer);

    // multiband process
    int freqValue1 = static_cast<int> (*apvts.getRawParameterValue (FREQ_ID1));
    int freqValue2 = static_cast<int> (*apvts.getRawParameterValue (FREQ_ID2));
    int freqValue3 = static_cast<int> (*apvts.getRawParameterValue (FREQ_ID3));

    bool lineState1 = static_cast<bool> (*apvts.getRawParameterValue (LINE_STATE_ID1));
    bool lineState2 = static_cast<bool> (*apvts.getRawParameterValue (LINE_STATE_ID2));
    bool lineState3 = static_cast<bool> (*apvts.getRawParameterValue (LINE_STATE_ID3));

    // sort freq
    std::array<int, 3> freqArray = { 0, 0, 0 };
    int count = 0;
    if (lineState1)
        freqArray[count++] = freqValue1;
    if (lineState2)
        freqArray[count++] = freqValue2;
    if (lineState3)
        freqArray[count++] = freqValue3;

    //TODO: Temp fix. Should not set lineNum in processblock every time.
    lineNum = count;

    std::sort (freqArray.begin(), freqArray.begin() + count);

    freqValue1 = freqArray[0];
    freqValue2 = freqArray[1];
    freqValue3 = freqArray[2];

    auto numSamples = buffer.getNumSamples();
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        mBuffers[i].makeCopyOf(buffer);
        multibandEnables[i] = *apvts.getRawParameterValue("multibandEnable" + std::to_string(i + 1));
        multibandSolos[i] = *apvts.getRawParameterValue("multibandSolo" + std::to_string(i + 1));
    }

    // set channels
    int leftChannelId;
    int rightChannelId;
    if (totalNumInputChannels == 2)
    {
        leftChannelId = 0;
        rightChannelId = 1;
    }
    else
    {
        leftChannelId = 0;
        rightChannelId = 0;
    }

    if (lineNum >= 0)
    {
        auto multibandBlock1 = juce::dsp::AudioBlock<float> (mBuffers[0]);
        auto context1 = juce::dsp::ProcessContextReplacing<float> (multibandBlock1);

        if (lineNum > 0)
        {
            lowpasses[0].setCutoffFrequency (static_cast<float>(freqValue1));
            lowpasses[0].process (context1);
        }

        mDryBuffer.makeCopyOf (mBuffers[0]);

        setLeftRightMeterRMSValues (mBuffers[0], mInputLeftSmoothedBands[0], mInputRightSmoothedBands[0]);

        if (multibandEnables[0])
        {
            processOneBand (mBuffers[0], context1, MODE_ID1, DRIVE_ID1, SAFE_ID1, EXTREME_ID1, BIAS_ID1, REC_ID1, overdrives[0], OUTPUT_ID1, gainProcessors[0], COMP_THRESH_ID1, COMP_RATIO_ID1, compressorProcessors[0], totalNumInputChannels, recSmoothers[0], outputSmoothers[0], MIX_ID1, dryWetMixer1, WIDTH_ID1, widthProcessors[0], dcFilters[0], WIDTH_BYPASS_ID1, COMP_BYPASS_ID1);
        }

        setLeftRightMeterRMSValues (mBuffers[0], mOutputLeftSmoothedBands[0], mOutputRightSmoothedBands[0]);
    }

    if (lineNum >= 1)
    {
        auto multibandBlock2 = juce::dsp::AudioBlock<float> (mBuffers[1]);
        auto context2 = juce::dsp::ProcessContextReplacing<float> (multibandBlock2);
        highpasses[0].setCutoffFrequency (static_cast<float>(freqValue1));
        highpasses[0].process (context2);

        if (lineNum > 1)
        {
            lowpasses[1].setCutoffFrequency (static_cast<float>(freqValue2));
            lowpasses[1].process (context2);
        }

        mDryBuffer.addFrom (leftChannelId, 0, mBuffers[1], leftChannelId, 0, numSamples);
        mDryBuffer.addFrom (rightChannelId, 0, mBuffers[1], rightChannelId, 0, numSamples);

        setLeftRightMeterRMSValues (mBuffers[1], mInputLeftSmoothedBands[1], mInputRightSmoothedBands[1]);

        if (multibandEnables[1])
        {
            processOneBand (mBuffers[1], context2, MODE_ID2, DRIVE_ID2, SAFE_ID2, EXTREME_ID2, BIAS_ID2, REC_ID2, overdrives[1], OUTPUT_ID2, gainProcessors[1], COMP_THRESH_ID2, COMP_RATIO_ID2, compressorProcessors[1], totalNumInputChannels, recSmoothers[2], outputSmoothers[2], MIX_ID2, dryWetMixer2, WIDTH_ID2, widthProcessors[2], dcFilters[2], WIDTH_BYPASS_ID2, COMP_BYPASS_ID2);
        }

        setLeftRightMeterRMSValues (mBuffers[1], mOutputLeftSmoothedBands[1], mOutputRightSmoothedBands[1]);
    }

    if (lineNum >= 2)
    {
        auto multibandBlock3 = juce::dsp::AudioBlock<float> (mBuffers[2]);
        auto context3 = juce::dsp::ProcessContextReplacing<float> (multibandBlock3);
        highpasses[1].setCutoffFrequency (static_cast<float>(freqValue2));
        highpasses[1].process (context3);

        if (lineNum > 2)
        {
            lowpasses[2].setCutoffFrequency (static_cast<float>(freqValue3));
            lowpasses[2].process (context3);
        }

        mDryBuffer.addFrom (leftChannelId, 0, mBuffers[2], leftChannelId, 0, numSamples);
        mDryBuffer.addFrom (rightChannelId, 0, mBuffers[2], rightChannelId, 0, numSamples);

        setLeftRightMeterRMSValues (mBuffers[2], mInputLeftSmoothedBands[2], mInputRightSmoothedBands[2]);

        if (multibandEnables[2])
        {
            processOneBand (mBuffers[2], context3, MODE_ID3, DRIVE_ID3, SAFE_ID3, EXTREME_ID3, BIAS_ID3, REC_ID3, overdrives[2], OUTPUT_ID3, gainProcessors[2], COMP_THRESH_ID3, COMP_RATIO_ID3, compressorProcessors[2], totalNumInputChannels, recSmoothers[2], outputSmoothers[2], MIX_ID3, dryWetMixer3, WIDTH_ID3, widthProcessors[2], dcFilters[2], WIDTH_BYPASS_ID3, COMP_BYPASS_ID3);
        }

        setLeftRightMeterRMSValues (mBuffers[2], mOutputLeftSmoothedBands[2], mOutputRightSmoothedBands[2]);
    }

    if (lineNum == 3)
    {
        auto multibandBlock4 = juce::dsp::AudioBlock<float> (mBuffers[3]);
        auto context4 = juce::dsp::ProcessContextReplacing<float> (multibandBlock4);
        highpasses[2].setCutoffFrequency (static_cast<float>(freqValue3));
        highpasses[2].process (context4);

        setLeftRightMeterRMSValues (mBuffers[3], mInputLeftSmoothedBands[3], mInputRightSmoothedBands[3]);

        mDryBuffer.addFrom (leftChannelId, 0, mBuffers[3], leftChannelId, 0, numSamples);
        mDryBuffer.addFrom (rightChannelId, 0, mBuffers[3], rightChannelId, 0, numSamples);

        if (multibandEnables[3])
        {
            processOneBand (mBuffers[3], context4, MODE_ID4, DRIVE_ID4, SAFE_ID4, EXTREME_ID4, BIAS_ID4, REC_ID4, overdrives[3], OUTPUT_ID4, gainProcessors[3], COMP_THRESH_ID4, COMP_RATIO_ID4, compressorProcessors[3], totalNumInputChannels, recSmoothers[3], outputSmoothers[3], MIX_ID4, dryWetMixer4, WIDTH_ID4, widthProcessors[3], dcFilters[3], WIDTH_BYPASS_ID4, COMP_BYPASS_ID4);
        }

        setLeftRightMeterRMSValues (mBuffers[3], mOutputLeftSmoothedBands[3], mOutputRightSmoothedBands[3]);
    }

    // set latency if all enable buttons are on, else set to 0
    if (lineNum == 0 && ! multibandEnables[0])
    {
        mLatency = 0;
        setLatencySamples (static_cast<int>(mLatency));
    }
    if (lineNum == 1 && ! multibandEnables[0] && ! multibandEnables[1])
    {
        mLatency = 0;
        setLatencySamples (static_cast<int>(mLatency));
    }
    if (lineNum == 2 && ! multibandEnables[0] && ! multibandEnables[1] && ! multibandEnables[2])
    {
        mLatency = 0;
        setLatencySamples (static_cast<int>(mLatency));
    }
    if (lineNum == 3 && ! multibandEnables[0] && ! multibandEnables[1] && ! multibandEnables[2] && ! multibandEnables[3])
    {
        mLatency = 0;
        setLatencySamples (static_cast<int>(mLatency));
    }

    buffer.clear();

    if (!shouldSetBlackMask (0))
    {
        buffer.addFrom (leftChannelId, 0, mBuffers[0], leftChannelId, 0, numSamples);
        buffer.addFrom (rightChannelId, 0, mBuffers[0], rightChannelId, 0, numSamples);
    }
    if (! shouldSetBlackMask (1) && lineNum >= 1)
    {
        buffer.addFrom (leftChannelId, 0, mBuffers[1], leftChannelId, 0, numSamples);
        buffer.addFrom (rightChannelId, 0, mBuffers[1], rightChannelId, 0, numSamples);
    }
    if (! shouldSetBlackMask (2) && lineNum >= 2)
    {
        buffer.addFrom (leftChannelId, 0, mBuffers[2], leftChannelId, 0, numSamples);
        buffer.addFrom (rightChannelId, 0, mBuffers[2], rightChannelId, 0, numSamples);
    }
    if (! shouldSetBlackMask (3) && lineNum == 3)
    {
        buffer.addFrom (leftChannelId, 0, mBuffers[3], leftChannelId, 0, numSamples);
        buffer.addFrom (rightChannelId, 0, mBuffers[3], rightChannelId, 0, numSamples);
    }

    // downsample
    if (*apvts.getRawParameterValue (DOWNSAMPLE_BYPASS_ID))
    {
        int rateDivide = static_cast<int> (*apvts.getRawParameterValue (DOWNSAMPLE_ID));
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer (channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                //int rateDivide = (distortionProcessor.controls.drive - 1) / 63.f * 99.f + 1; //range(1,100)
                if (rateDivide > 1)
                {
                    if (sample % rateDivide != 0)
                        channelData[sample] = channelData[sample - sample % rateDivide];
                }
            }
        }
    }

    if (*apvts.getRawParameterValue (FILTER_BYPASS_ID))
    {
        updateFilter();
        auto leftBlock = block.getSingleChannelBlock (leftChannelId);
        auto rightBlock = block.getSingleChannelBlock (rightChannelId);
        leftChain.process (juce::dsp::ProcessContextReplacing<float> (leftBlock));
        rightChain.process (juce::dsp::ProcessContextReplacing<float> (rightBlock));
    }

    // global process
    auto globalBlock = juce::dsp::AudioBlock<float> (buffer);
    auto contextGlobal = juce::dsp::ProcessContextReplacing<float> (globalBlock);
    
    if (*apvts.getRawParameterValue (LIMITER_BYPASS_ID))
    {
        processLimiter(contextGlobal, LIMITER_THRESH_ID, LIMITER_RELEASE_ID, limiterProcessorGlobal);
    }
    
    processGain(contextGlobal, OUTPUT_ID, gainProcessorGlobal);

    // mix dry wet
    mixDryWet (mDryBuffer, buffer, MIX_ID, dryWetMixerGlobal, mLatency);

    // Spectrum
    mWetBuffer.makeCopyOf(buffer); 
    pushDataToFFT();

    // VU output meter
    setLeftRightMeterRMSValues(buffer, mOutputLeftSmoothedGlobal, mOutputRightSmoothedGlobal);

    mDryBuffer.clear();
}

juce::AudioProcessorEditor* HayesDistortionAudioProcessor::createEditor()
{
    return new HayesDistortionAudioProcessorEditor (*this);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HayesDistortionAudioProcessor();
}

// Filter selection
void updateCoefficients (CoefficientsPtr& old, const CoefficientsPtr& replacements)
{
    *old = *replacements;
}

ChainSettings getChainSettings (juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;

    settings.lowCutFreq = apvts.getRawParameterValue (LOWCUT_FREQ_ID)->load();
    settings.lowCutQuality = apvts.getRawParameterValue (LOWCUT_Q_ID)->load();
    settings.lowCutGainInDecibels = apvts.getRawParameterValue (LOWCUT_GAIN_ID)->load();
    settings.highCutFreq = apvts.getRawParameterValue (HIGHCUT_FREQ_ID)->load();
    settings.highCutQuality = apvts.getRawParameterValue (HIGHCUT_Q_ID)->load();
    settings.highCutGainInDecibels = apvts.getRawParameterValue (HIGHCUT_GAIN_ID)->load();
    settings.peakFreq = apvts.getRawParameterValue (PEAK_FREQ_ID)->load();
    settings.peakGainInDecibels = apvts.getRawParameterValue (PEAK_GAIN_ID)->load();
    settings.peakQuality = apvts.getRawParameterValue (PEAK_Q_ID)->load();
    settings.lowCutSlope = static_cast<Slope> (apvts.getRawParameterValue (LOWCUT_SLOPE_ID)->load());
    settings.highCutSlope = static_cast<Slope> (apvts.getRawParameterValue (HIGHCUT_SLOPE_ID)->load());

    settings.lowCutBypassed = apvts.getRawParameterValue (LOWCUT_BYPASSED_ID)->load(); // > 0.5f;
    settings.peakBypassed = apvts.getRawParameterValue (PEAK_BYPASSED_ID)->load(); // > 0.5f;
    settings.highCutBypassed = apvts.getRawParameterValue (HIGHCUT_BYPASSED_ID)->load(); // > 0.5f;

    return settings;
}

CoefficientsPtr makePeakFilter (const ChainSettings& chainSettings, double sampleRate)
{
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate,
                                                                chainSettings.peakFreq,
                                                                chainSettings.peakQuality,
                                                                juce::Decibels::decibelsToGain (chainSettings.peakGainInDecibels));
}

CoefficientsPtr makeLowcutQFilter (const ChainSettings& chainSettings, double sampleRate)
{
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate,
                                                                chainSettings.lowCutFreq,
                                                                chainSettings.lowCutQuality,
                                                                juce::Decibels::decibelsToGain (chainSettings.lowCutGainInDecibels));
}

CoefficientsPtr makeHighcutQFilter (const ChainSettings& chainSettings, double sampleRate)
{
    return juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate,
                                                                chainSettings.highCutFreq,
                                                                chainSettings.highCutQuality,
                                                                juce::Decibels::decibelsToGain (chainSettings.highCutGainInDecibels));
}

void HayesDistortionAudioProcessor::updatePeakFilter (const ChainSettings& chainSettings)
{
    auto peakCoefficients = makePeakFilter (chainSettings, getSampleRate());

    leftChain.setBypassed<ChainPositions::Peak> (chainSettings.peakBypassed);
    rightChain.setBypassed<ChainPositions::Peak> (chainSettings.peakBypassed);
    updateCoefficients (leftChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    updateCoefficients (rightChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
}

void HayesDistortionAudioProcessor::updateLowCutFilters (const ChainSettings& chainSettings)
{
    auto cutCoefficients = makeLowCutFilter (chainSettings, getSampleRate());
    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();

    auto lowcutQCoefficients = makeLowcutQFilter (chainSettings, getSampleRate());

    leftChain.setBypassed<ChainPositions::LowCut> (chainSettings.lowCutBypassed);
    rightChain.setBypassed<ChainPositions::LowCut> (chainSettings.lowCutBypassed);
    leftChain.setBypassed<ChainPositions::LowCutQ> (chainSettings.lowCutBypassed);
    rightChain.setBypassed<ChainPositions::LowCutQ> (chainSettings.lowCutBypassed);

    updateCutFilter (rightLowCut, cutCoefficients, chainSettings.lowCutSlope);
    updateCutFilter (leftLowCut, cutCoefficients, chainSettings.lowCutSlope);

    updateCoefficients (leftChain.get<ChainPositions::LowCutQ>().coefficients, lowcutQCoefficients);
    updateCoefficients (rightChain.get<ChainPositions::LowCutQ>().coefficients, lowcutQCoefficients);
}

void HayesDistortionAudioProcessor::updateHighCutFilters (const ChainSettings& chainSettings)
{
    auto highCutCoefficients = makeHighCutFilter (chainSettings, getSampleRate());
    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();

    auto highcutQCoefficients = makeHighcutQFilter (chainSettings, getSampleRate());

    leftChain.setBypassed<ChainPositions::HighCut> (chainSettings.highCutBypassed);
    rightChain.setBypassed<ChainPositions::HighCut> (chainSettings.highCutBypassed);

    updateCutFilter (leftHighCut, highCutCoefficients, chainSettings.highCutSlope);
    updateCutFilter (rightHighCut, highCutCoefficients, chainSettings.highCutSlope);

    updateCoefficients (leftChain.get<ChainPositions::HighCutQ>().coefficients, highcutQCoefficients);
    updateCoefficients (rightChain.get<ChainPositions::HighCutQ>().coefficients, highcutQCoefficients);
}

void HayesDistortionAudioProcessor::updateFilter()
{
    auto chainSettings = getChainSettings (apvts);
    updateLowCutFilters (chainSettings);
    updatePeakFilter (chainSettings);
    updateHighCutFilters (chainSettings);
}

bool HayesDistortionAudioProcessor::isSilent(juce::AudioBuffer<float> buffer)
{
    if (buffer.getMagnitude (0, buffer.getNumSamples()) == 0)
        return true;
    else
        return false;
}

void HayesDistortionAudioProcessor::setHistoryArray(int bandIndex)
{
    for (int channel = 0; channel < getTotalNumOutputChannels(); ++channel)
    {
        int bufferSamples = 0;
        float* channelData;
        if (bandIndex >= 0 && bandIndex <= 3)
        {
            channelData = mBuffers[bandIndex].getWritePointer (channel);
            bufferSamples = mBuffers[bandIndex].getNumSamples();
        }
        else
        {
            channelData = mWetBuffer.getWritePointer (channel);
            bufferSamples = mWetBuffer.getNumSamples();
        }

        for (int sample = 0; sample < bufferSamples; ++sample)
        {
            // mDelay is delayed clean signal
            if (sample % 10 == 0)
            {
                if (channel == 0)
                {
                    historyArrayL.add (channelData[sample]);
                    if (historyArrayL.size() > historyLength)
                    {
                        historyArrayL.remove (0);
                    }
                }
                else if (channel == 1)
                {
                    historyArrayR.add (channelData[sample]);
                    if (historyArrayR.size() > historyLength)
                    {
                        historyArrayR.remove (0);
                    }
                }
            }
        }
    }
}

juce::Array<float> HayesDistortionAudioProcessor::getHistoryArrayL()
{
    return historyArrayL;
}

juce::Array<float> HayesDistortionAudioProcessor::getHistoryArrayR()
{
    return historyArrayR;
}

float* HayesDistortionAudioProcessor::getFFTData()
{
    return spectrum_processor.fftData;
}

int HayesDistortionAudioProcessor::getNumBins()
{
    return spectrum_processor.numBins;
}

int HayesDistortionAudioProcessor::getFFTSize()
{
    return spectrum_processor.fftSize;
}

bool HayesDistortionAudioProcessor::isFFTBlockReady()
{
    return spectrum_processor.nextFFTBlockReady;
}

void HayesDistortionAudioProcessor::pushDataToFFT()
{
    if (mWetBuffer.getNumChannels() > 0)
    {
        auto* channelData = mWetBuffer.getReadPointer (0);

        for (auto i = 0; i < mWetBuffer.getNumSamples(); ++i)
            spectrum_processor.pushNextSampleIntoFifo (channelData[i]);
    }
}

void HayesDistortionAudioProcessor::processFFT (float* tempFFTData)
{
    spectrum_processor.doProcessing (tempFFTData);
    spectrum_processor.nextFFTBlockReady = false;
}

float HayesDistortionAudioProcessor::safeMode (float drive, juce::AudioBuffer<float>& buffer, juce::String safeID)
{
    // protection
    drive = drive * 6.5f / 100.0f;
    float powerDrive = powf (2, drive);

    float sampleMaxValue = 0;
    sampleMaxValue = buffer.getMagnitude (0, buffer.getNumSamples());

    bool isSafeModeOn = *apvts.getRawParameterValue (safeID);

    float newDrive = 0.0f;
    if (isSafeModeOn && sampleMaxValue * powerDrive > 2.0f)
    {
        newDrive = 2.0f / sampleMaxValue + 0.1f * std::log2f (powerDrive);
        //newDrive = 2.0f / sampleMaxValue + 0.1 * drive;
    }
    else
    {
        newDrive = powerDrive;
    }

    if (drive == 0 || sampleMaxValue <= 0.001f)
    {
        setReductionPrecent (safeID, 1);
    }
    else
    {
        setReductionPrecent (safeID, std::log2f (newDrive) / drive);
    }
    setSampleMaxValue (safeID, sampleMaxValue);

    return newDrive;
}

bool HayesDistortionAudioProcessor::shouldSetBlackMask (int index)
{
    bool selfBandIsOn = getSoloStateFromIndex (index);
    bool otherBandSoloIsOn = false;

    for (int i = 0; i <= lineNum; i++)
    {
        if (i == index)
            continue;
        if (getSoloStateFromIndex (i))
        {
            otherBandSoloIsOn = true;
            break;
        }
    }
    return (! selfBandIsOn && otherBandSoloIsOn);
}

bool HayesDistortionAudioProcessor::getSoloStateFromIndex (int index)
{
    if (index == 0)
        return *apvts.getRawParameterValue (BAND_SOLO_ID1);
    else if (index == 1)
        return *apvts.getRawParameterValue (BAND_SOLO_ID2);
    else if (index == 2)
        return *apvts.getRawParameterValue (BAND_SOLO_ID3);
    else if (index == 3)
        return *apvts.getRawParameterValue (BAND_SOLO_ID4);
    else
        jassertfalse;
    return false;
}

void HayesDistortionAudioProcessor::processOneBand (juce::AudioBuffer<float>& bandBuffer, juce::dsp::ProcessContextReplacing<float> context, juce::String modeID, juce::String driveID, juce::String safeID, juce::String extremeID, juce::String biasID, juce::String recID, juce::dsp::ProcessorChain<GainProcessor, BiasProcessor, DriveProcessor, juce::dsp::WaveShaper<float, std::function<float (float)>>, BiasProcessor>& overdrive, juce::String outputID, GainProcessor& gainProcessor, juce::String threshID, juce::String ratioID, CompressorProcessor& compressorProcessor, int totalNumInputChannels, juce::SmoothedValue<float>& recSmoother, juce::SmoothedValue<float>& /*outputSmoother*/, juce::String mixID, juce::dsp::DryWetMixer<float>& dryWetMixer, juce::String widthID, WidthProcessor widthProcessor, DCFilter& dcFilter, juce::String widthBypassID, juce::String compBypassID)
{
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf (bandBuffer);

    // distortion process
    processDistortion (bandBuffer, modeID, driveID, safeID, extremeID, biasID, recID, overdrive, dcFilter);

    // normalize wave center position
    normalize (modeID, bandBuffer, totalNumInputChannels, recSmoother, outputSmoothers[0]);

    // width process
    if (*apvts.getRawParameterValue (widthBypassID) && totalNumInputChannels == 2)
    {
        float* channeldataL;
        float* channeldataR;
        float width = *apvts.getRawParameterValue (widthID);
        channeldataL = bandBuffer.getWritePointer (0);
        channeldataR = bandBuffer.getWritePointer (1);
        widthProcessor.process (channeldataL, channeldataR, width, bandBuffer.getNumSamples());
    }

    // compressor process
    if (*apvts.getRawParameterValue (compBypassID))
        processCompressor (context, threshID, ratioID, compressorProcessor);

    // gain process
    processGain (context, outputID, gainProcessor);

    // mix process
    mixDryWet (dryBuffer, bandBuffer, mixID, dryWetMixer, mLatency);
}

void HayesDistortionAudioProcessor::processDistortion (juce::AudioBuffer<float>& bandBuffer, juce::String modeID, juce::String driveID, juce::String safeID, juce::String extremeID, juce::String biasID, juce::String recID, juce::dsp::ProcessorChain<GainProcessor, BiasProcessor, DriveProcessor, juce::dsp::WaveShaper<float, std::function<float (float)>>, BiasProcessor>& overdrive, DCFilter& dcFilter)
{
    // oversampling
    juce::dsp::AudioBlock<float> blockInput (bandBuffer);
    juce::dsp::AudioBlock<float> blockOutput;
    int num;
    if (modeID == MODE_ID1)
        num = 0;
    else if (modeID == MODE_ID2)
        num = 1;
    else if (modeID == MODE_ID3)
        num = 2;
    else if (modeID == MODE_ID4)
        num = 3;
    else
        num = -1;

    blockOutput = blockInput.getSubBlock (0, bandBuffer.getNumSamples());
    mLatency = 0;
    setLatencySamples (0);

    // band process
    auto context = juce::dsp::ProcessContextReplacing<float> (blockOutput);

    // get parameters from sliders
    int mode = static_cast<int> (*apvts.getRawParameterValue (modeID));
    float driveValue = static_cast<float> (*apvts.getRawParameterValue (driveID));
    float biasValue = static_cast<float> (*apvts.getRawParameterValue (biasID));
    float recValue = static_cast<float> (*apvts.getRawParameterValue (recID));

    if (static_cast<bool>(*apvts.getRawParameterValue (extremeID)))
    {
        driveValue = log2 (10.0f) * driveValue;
    }
    float newDrive = safeMode (driveValue, bandBuffer, safeID);

    if (driveID == DRIVE_ID1)
        newDrive1 = newDrive;
    else if (driveID == DRIVE_ID2)
        newDrive2 = newDrive;
    else if (driveID == DRIVE_ID3)
        newDrive3 = newDrive;
    else if (driveID == DRIVE_ID4)
        newDrive4 = newDrive;

    auto& gainUp = overdrive.get<0>();
    gainUp.setGainLinear (newDrive);
    gainUp.setRampDurationSeconds (0.05f);

    float maxValue;
    if (getTotalNumInputChannels() == 2)
        maxValue = juce::jmax (bandBuffer.getRMSLevel (0, 0, bandBuffer.getNumSamples()), bandBuffer.getRMSLevel (1, 0, bandBuffer.getNumSamples()));
    else
        maxValue = bandBuffer.getRMSLevel (0, 0, bandBuffer.getNumSamples());

    if (maxValue < 0.00001f)
        biasValue = 0;

    auto& bias = overdrive.get<1>();
    bias.setBias (biasValue); // -1,1
    bias.setRampDurationSeconds (0.05f);

    auto& waveShaper = overdrive.get<2>();

    switch (mode)
    {
        case 0:
            waveShaper.functionToUse = waveshaping::arctanSoftClipping;
            break;
        case 1:
            waveShaper.functionToUse = waveshaping::expSoftClipping;
            break;
        case 2:
            waveShaper.functionToUse = waveshaping::tanhSoftClipping;
            break;
        case 3:
            waveShaper.functionToUse = waveshaping::cubicSoftClipping;
            break;
        case 4:
            waveShaper.functionToUse = waveshaping::hardClipping;
            break;
        case 5:
            waveShaper.functionToUse = waveshaping::sausageFattener;
            break;
        case 6:
            waveShaper.functionToUse = waveshaping::sinFoldback;
            break;
        case 7:
            waveShaper.functionToUse = waveshaping::linFoldback;
            break;
        case 8:
            waveShaper.functionToUse = waveshaping::limitClip;
            break;
        case 9:
            waveShaper.functionToUse = waveshaping::singleSinClip;
            break;
        case 10:
            waveShaper.functionToUse = waveshaping::logicClip;
            break;
        case 11:
            waveShaper.functionToUse = waveshaping::tanclip;
            break;
    }

    auto& waveShaper2 = overdrive.get<3>();

    waveShaper2.functionToUse = [recValue] (float input)
    {
        if (input < 0)
        {
            input *= (0.5f - recValue) * 2.0f;
        }
        return input;
    };

    auto& bias2 = overdrive.get<4>();
    bias2.setBias (-biasValue); // -1,1
    bias2.setRampDurationSeconds (0.05f);

    overdrive.process (context);

    dcFilter.state = juce::dsp::IIR::Coefficients<float>::makeHighPass (getSampleRate(), 20.0);
}

void HayesDistortionAudioProcessor::processCompressor (juce::dsp::ProcessContextReplacing<float> context, juce::String threshID, juce::String ratioID, CompressorProcessor& compressor)
{
    float ratio = *apvts.getRawParameterValue (ratioID);
    float thresh = *apvts.getRawParameterValue (threshID);
    compressor.setThreshold (thresh);
    compressor.setRatio (ratio);
    compressor.process (context);
}

void HayesDistortionAudioProcessor::processLimiter (juce::dsp::ProcessContextReplacing<float> context, juce::String limiterThreshID, juce::String limiterReleaseID, LimiterProcessor& limiterProcessor)
{
    float limiterThreshValue = *apvts.getRawParameterValue (limiterThreshID);
    float limiterReleaseValue = *apvts.getRawParameterValue (limiterReleaseID);

    limiterProcessor.setThreshold(-limiterThreshValue);
    limiterProcessor.setRelease(limiterReleaseValue);
    limiterProcessor.process (context);
}

void HayesDistortionAudioProcessor::processGain (juce::dsp::ProcessContextReplacing<float> context, juce::String outputID, GainProcessor& gainProcessor)
{
    float outputValue = *apvts.getRawParameterValue (outputID);
    gainProcessor.setGainDecibels (outputValue);
    gainProcessor.setRampDurationSeconds (0.05f);
    gainProcessor.process (context);
}

void HayesDistortionAudioProcessor::mixDryWet (juce::AudioBuffer<float>& dryBuffer, juce::AudioBuffer<float>& wetBuffer, juce::String mixID, juce::dsp::DryWetMixer<float>& dryWetMixer, float latency)
{
    float mixValue = static_cast<float> (*apvts.getRawParameterValue (mixID));
    auto dryBlock = juce::dsp::AudioBlock<float> (dryBuffer);
    auto wetBlock = juce::dsp::AudioBlock<float> (wetBuffer);
    dryWetMixer.setMixingRule (juce::dsp::DryWetMixingRule::linear);
    dryWetMixer.pushDrySamples (dryBlock);
    if (*apvts.getRawParameterValue (HQ_ID))
    {
        dryWetMixer.setWetLatency (latency);
    }
    else
    {
        dryWetMixer.setWetLatency (0);
    }
    dryWetMixer.setWetMixProportion (mixValue);
    dryWetMixer.mixWetSamples (wetBlock);
}

void HayesDistortionAudioProcessor::normalize (juce::String modeID, juce::AudioBuffer<float>& buffer, int totalNumInputChannels, juce::SmoothedValue<float>& recSmoother, juce::SmoothedValue<float>& outputSmoother)
{
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        juce::Range<float> range = buffer.findMinMax (channel, 0, buffer.getNumSamples());
        float min = range.getStart();
        float max = range.getEnd();

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // centralization
            if (/*mode == diodemode  || */ recSmoother.getNextValue() > 0)
            {
                centralSmoother.setTargetValue ((max + min) / 2.0f);
                channelData[sample] = channelData[sample] - centralSmoother.getNextValue();
            }
            // output control
            channelData[sample] *= outputSmoother.getNextValue();
        }
    }
}

int HayesDistortionAudioProcessor::getSavedWidth() const
{
    return editorWidth;
}

int HayesDistortionAudioProcessor::getSavedHeight() const
{
    return editorHeight;
}

void HayesDistortionAudioProcessor::setSavedWidth (const int width)
{
    editorWidth = width;
}

void HayesDistortionAudioProcessor::setSavedHeight (const int height)
{
    editorHeight = height;
}

bool HayesDistortionAudioProcessor::getBypassedState()
{
    return isBypassed;
}

void HayesDistortionAudioProcessor::setLineNum()
{
    // maybe this method is useless, or delete "lineNum = count" in processBlock
    int num = 0;

    if (*apvts.getRawParameterValue (LINE_STATE_ID1))
    {
        num += 1;
    }
    if (*apvts.getRawParameterValue (LINE_STATE_ID2))
    {
        num += 1;
    }
    if (*apvts.getRawParameterValue (LINE_STATE_ID3))
    {
        num += 1;
    }

    this->lineNum = num;
}

// VU meters
void HayesDistortionAudioProcessor::setLeftRightMeterRMSValues (juce::AudioBuffer<float> buffer, float& leftOutValue, float& rightOutValue)
{
    float absInputLeftValue;
    float absInputRightValue;
    if (getTotalNumInputChannels() == 2)
    {
        absInputLeftValue = fabs (buffer.getRMSLevel (0, 0, buffer.getNumSamples()));
        absInputRightValue = fabs (buffer.getRMSLevel (1, 0, buffer.getNumSamples()));
    }
    else
    {
        absInputLeftValue = fabs (buffer.getRMSLevel (0, 0, buffer.getNumSamples()));
        absInputRightValue = absInputLeftValue;
    }
    // smooth value
    leftOutValue = static_cast<float>(SMOOTH_COEFF) * (leftOutValue - absInputLeftValue) + absInputLeftValue;
    rightOutValue = static_cast<float>(SMOOTH_COEFF) * (rightOutValue - absInputRightValue) + absInputRightValue;
}

float HayesDistortionAudioProcessor::getInputMeterRMSLevel (int channel, juce::String bandName)
{
    float outputValue = 0;
    if (bandName == "Global")
    {
        if (channel == 0)
            outputValue = dBToNormalizedGain (mInputLeftSmoothedGlobal);
        else
            outputValue = dBToNormalizedGain (mInputRightSmoothedGlobal);
    }
    else if (bandName == "Band1")
    {
        if (channel == 0)
            outputValue = dBToNormalizedGain (mInputLeftSmoothedBands[0]);
        else
            outputValue = dBToNormalizedGain (mInputRightSmoothedBands[0]);
    }
    else if (bandName == "Band2")
    {
        if (channel == 0)
            outputValue = dBToNormalizedGain (mInputLeftSmoothedBands[1]);
        else
            outputValue = dBToNormalizedGain (mInputRightSmoothedBands[1]);
    }
    else if (bandName == "Band3")
    {
        if (channel == 0)
            outputValue = dBToNormalizedGain (mInputLeftSmoothedBands[2]);
        else
            outputValue = dBToNormalizedGain (mInputRightSmoothedBands[2]);
    }
    else if (bandName == "Band4")
    {
        if (channel == 0)
            outputValue = dBToNormalizedGain (mInputLeftSmoothedBands[3]);
        else
            outputValue = dBToNormalizedGain (mInputRightSmoothedBands[3]);
    }
    else
    {
        jassertfalse;
    }
    return outputValue;
}

float HayesDistortionAudioProcessor::getOutputMeterRMSLevel (int channel, juce::String bandName)
{
    float outputValue = 0;
    if (bandName == "Global")
    {
        if (channel == 0)
            outputValue = dBToNormalizedGain (mOutputLeftSmoothedGlobal);
        else
            outputValue = dBToNormalizedGain (mOutputRightSmoothedGlobal);
    }
    else if (bandName == "Band1")
    {
        if (channel == 0)
            outputValue = dBToNormalizedGain (mOutputLeftSmoothedBands[0]);
        else
            outputValue = dBToNormalizedGain (mOutputRightSmoothedBands[0]);
    }
    else if (bandName == "Band2")
    {
        if (channel == 0)
            outputValue = dBToNormalizedGain (mOutputLeftSmoothedBands[1]);
        else
            outputValue = dBToNormalizedGain (mOutputRightSmoothedBands[1]);
    }
    else if (bandName == "Band3")
    {
        if (channel == 0)
            outputValue = dBToNormalizedGain (mOutputLeftSmoothedBands[2]);
        else
            outputValue = dBToNormalizedGain (mOutputRightSmoothedBands[2]);
    }
    else if (bandName == "Band4")
    {
        if (channel == 0)
            outputValue = dBToNormalizedGain (mOutputLeftSmoothedBands[3]);
        else
            outputValue = dBToNormalizedGain (mOutputRightSmoothedBands[3]);
    }
    else
    {
        jassertfalse;
    }
    return outputValue;
}

// drive lookandfeel
float HayesDistortionAudioProcessor::getReductionPrecent (juce::String safeId)
{
    if (safeId == SAFE_ID1)
        return mReductionPrecents[0];
    if (safeId == SAFE_ID2)
        return mReductionPrecents[1];
    if (safeId == SAFE_ID3)
        return mReductionPrecents[2];
    if (safeId == SAFE_ID4)
        return mReductionPrecents[3];
    return 0;
}

void HayesDistortionAudioProcessor::setReductionPrecent (juce::String safeId, float reductionPrecent)
{
    if (safeId == SAFE_ID1)
        mReductionPrecents[0] = reductionPrecent;
    if (safeId == SAFE_ID2)
        mReductionPrecents[1] = reductionPrecent;
    if (safeId == SAFE_ID3)
        mReductionPrecents[2] = reductionPrecent;
    if (safeId == SAFE_ID4)
        mReductionPrecents[3] = reductionPrecent;
}

float HayesDistortionAudioProcessor::getSampleMaxValue (juce::String safeId)
{
    if (safeId == SAFE_ID1)
        return mSampleMaxValues[0];
    if (safeId == SAFE_ID2)
        return mSampleMaxValues[1];
    if (safeId == SAFE_ID3)
        return mSampleMaxValues[2];
    if (safeId == SAFE_ID4)
        return mSampleMaxValues[3];
    return 0;
}

void HayesDistortionAudioProcessor::setSampleMaxValue (juce::String safeId, float sampleMaxValue)
{
    if (safeId == SAFE_ID1)
        mSampleMaxValues[0] = sampleMaxValue;
    if (safeId == SAFE_ID2)
        mSampleMaxValues[1] = sampleMaxValue;
    if (safeId == SAFE_ID3)
        mSampleMaxValues[2] = sampleMaxValue;
    if (safeId == SAFE_ID4)
        mSampleMaxValues[3] = sampleMaxValue;
}

juce::AudioProcessorValueTreeState::ParameterLayout HayesDistortionAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    int versionNum = 1;
    using PBool = juce::AudioParameterBool;
    using PInt = juce::AudioParameterInt;
    using PFloat = juce::AudioParameterFloat;
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { HQ_ID, versionNum }, HQ_NAME, false));

    parameters.push_back (std::make_unique<PInt> (juce::ParameterID { MODE_ID1, versionNum }, MODE_NAME1, 0, 11, 0));
    parameters.push_back (std::make_unique<PInt> (juce::ParameterID { MODE_ID2, versionNum }, MODE_NAME2, 0, 11, 0));
    parameters.push_back (std::make_unique<PInt> (juce::ParameterID { MODE_ID3, versionNum }, MODE_NAME3, 0, 11, 0));
    parameters.push_back (std::make_unique<PInt> (juce::ParameterID { MODE_ID4, versionNum }, MODE_NAME4, 0, 11, 0));

    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { LINKED_ID1, versionNum }, LINKED_NAME1, true));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { LINKED_ID2, versionNum }, LINKED_NAME2, true));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { LINKED_ID3, versionNum }, LINKED_NAME3, true));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { LINKED_ID4, versionNum }, LINKED_NAME4, true));

    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { SAFE_ID1, versionNum }, SAFE_NAME1, true));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { SAFE_ID2, versionNum }, SAFE_NAME2, true));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { SAFE_ID3, versionNum }, SAFE_NAME3, true));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { SAFE_ID4, versionNum }, SAFE_NAME4, true));
    
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { EXTREME_ID1, versionNum }, EXTREME_NAME1, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { EXTREME_ID2, versionNum }, EXTREME_NAME2, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { EXTREME_ID3, versionNum }, EXTREME_NAME3, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { EXTREME_ID4, versionNum }, EXTREME_NAME4, false));

    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { DRIVE_ID1, versionNum }, DRIVE_NAME1, juce::NormalisableRange<float> (0.0f, 100.0f, 0.01f), 0.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { DRIVE_ID2, versionNum }, DRIVE_NAME2, juce::NormalisableRange<float> (0.0f, 100.0f, 0.01f), 0.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { DRIVE_ID3, versionNum }, DRIVE_NAME3, juce::NormalisableRange<float> (0.0f, 100.0f, 0.01f), 0.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { DRIVE_ID4, versionNum }, DRIVE_NAME4, juce::NormalisableRange<float> (0.0f, 100.0f, 0.01f), 0.0f));

    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { COMP_RATIO_ID1, versionNum }, COMP_RATIO_NAME1, juce::NormalisableRange<float> (1.0f, 20.0f, 0.1f), 1.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { COMP_RATIO_ID2, versionNum }, COMP_RATIO_NAME2, juce::NormalisableRange<float> (1.0f, 20.0f, 0.1f), 1.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { COMP_RATIO_ID3, versionNum }, COMP_RATIO_NAME3, juce::NormalisableRange<float> (1.0f, 20.0f, 0.1f), 1.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { COMP_RATIO_ID4, versionNum }, COMP_RATIO_NAME4, juce::NormalisableRange<float> (1.0f, 20.0f, 0.1f), 1.0f));

    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { COMP_THRESH_ID1, versionNum }, COMP_THRESH_NAME1, juce::NormalisableRange<float> (-48.0f, 0.0f, 0.1f), 0.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { COMP_THRESH_ID2, versionNum }, COMP_THRESH_NAME2, juce::NormalisableRange<float> (-48.0f, 0.0f, 0.1f), 0.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { COMP_THRESH_ID3, versionNum }, COMP_THRESH_NAME3, juce::NormalisableRange<float> (-48.0f, 0.0f, 0.1f), 0.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { COMP_THRESH_ID4, versionNum }, COMP_THRESH_NAME4, juce::NormalisableRange<float> (-48.0f, 0.0f, 0.1f), 0.0f));

    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { WIDTH_ID1, versionNum }, WIDTH_NAME1, juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { WIDTH_ID2, versionNum }, WIDTH_NAME2, juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { WIDTH_ID3, versionNum }, WIDTH_NAME3, juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { WIDTH_ID4, versionNum }, WIDTH_NAME4, juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));

    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { OUTPUT_ID1, versionNum }, OUTPUT_NAME1, juce::NormalisableRange<float> (-48.0f, 6.0f, 0.1f), 0.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { OUTPUT_ID2, versionNum }, OUTPUT_NAME2, juce::NormalisableRange<float> (-48.0f, 6.0f, 0.1f), 0.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { OUTPUT_ID3, versionNum }, OUTPUT_NAME3, juce::NormalisableRange<float> (-48.0f, 6.0f, 0.1f), 0.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { OUTPUT_ID4, versionNum }, OUTPUT_NAME4, juce::NormalisableRange<float> (-48.0f, 6.0f, 0.1f), 0.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { OUTPUT_ID, versionNum }, OUTPUT_NAME, juce::NormalisableRange<float> (-48.0f, 6.0f, 0.1f), 0.0f));

    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { MIX_ID1, versionNum }, MIX_NAME1, juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { MIX_ID2, versionNum }, MIX_NAME2, juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { MIX_ID3, versionNum }, MIX_NAME3, juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { MIX_ID4, versionNum }, MIX_NAME4, juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { MIX_ID, versionNum }, MIX_NAME, juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));

    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { BIAS_ID1, versionNum }, BIAS_NAME1, juce::NormalisableRange<float> (-1.0f, 1.0f, 0.01f), 0.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { BIAS_ID2, versionNum }, BIAS_NAME2, juce::NormalisableRange<float> (-1.0f, 1.0f, 0.01f), 0.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { BIAS_ID3, versionNum }, BIAS_NAME3, juce::NormalisableRange<float> (-1.0f, 1.0f, 0.01f), 0.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { BIAS_ID4, versionNum }, BIAS_NAME4, juce::NormalisableRange<float> (-1.0f, 1.0f, 0.01f), 0.0f));

    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { REC_ID1, versionNum }, REC_NAME1, juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { REC_ID2, versionNum }, REC_NAME2, juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { REC_ID3, versionNum }, REC_NAME3, juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { REC_ID4, versionNum }, REC_NAME4, juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

    juce::NormalisableRange<float> cutoffRange (20.0f, 20000.0f, 1.0f);
    cutoffRange.setSkewForCentre (1000.0f);
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { LOWCUT_FREQ_ID, versionNum }, LOWCUT_FREQ_NAME, cutoffRange, 20.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { LOWCUT_Q_ID, versionNum }, LOWCUT_Q_NAME, juce::NormalisableRange<float> (1.0f, 5.0f, 0.1f), 1.0f));

    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { HIGHCUT_FREQ_ID, versionNum }, HIGHCUT_FREQ_NAME, cutoffRange, 20000.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { HIGHCUT_Q_ID, versionNum }, HIGHCUT_Q_NAME, juce::NormalisableRange<float> (1.0f, 5.0f, 0.1f), 1.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { LOWCUT_GAIN_ID, versionNum }, LOWCUT_GAIN_NAME, juce::NormalisableRange<float> (-15.0f, 15.0f, 0.1f), 0.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { HIGHCUT_GAIN_ID, versionNum }, HIGHCUT_GAIN_NAME, juce::NormalisableRange<float> (-15.0f, 15.0f, 0.1f), 0.0f));

    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { PEAK_FREQ_ID, versionNum }, PEAK_FREQ_NAME, cutoffRange, 1000.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { PEAK_Q_ID, versionNum }, PEAK_Q_NAME, juce::NormalisableRange<float> (1.0f, 5.0f, 0.1f), 1.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { PEAK_GAIN_ID, versionNum }, PEAK_GAIN_NAME, juce::NormalisableRange<float> (-15.0f, 15.0f, 0.1f), 0.0f));

    parameters.push_back (std::make_unique<PInt> (juce::ParameterID { LOWCUT_SLOPE_ID, versionNum }, LOWCUT_SLOPE_NAME, 0, 3, 0));
    parameters.push_back (std::make_unique<PInt> (juce::ParameterID { HIGHCUT_SLOPE_ID, versionNum }, HIGHCUT_SLOPE_NAME, 0, 3, 0));

    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { LOWCUT_BYPASSED_ID, versionNum }, LOWCUT_BYPASSED_NAME, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { PEAK_BYPASSED_ID, versionNum }, PEAK_BYPASSED_NAME, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { HIGHCUT_BYPASSED_ID, versionNum }, HIGHCUT_BYPASSED_NAME, false));

    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { OFF_ID, versionNum }, OFF_NAME, true));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { PRE_ID, versionNum }, PRE_NAME, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { POST_ID, versionNum }, POST_NAME, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { LOW_ID, versionNum }, LOW_NAME, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { BAND_ID, versionNum }, BAND_NAME, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { HIGH_ID, versionNum }, HIGH_NAME, true));
    
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { DOWNSAMPLE_ID, versionNum }, DOWNSAMPLE_NAME, juce::NormalisableRange<float> (1.0f, 64.0f, 0.01f), 1.0f));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { LIMITER_THRESH_ID, versionNum }, LIMITER_THRESH_NAME, juce::NormalisableRange<float> (-24.0f, 0.0f, 0.1f), 0.0f));
    juce::NormalisableRange<float> limiterReleaseRange (0.01f, 3000.0f, 0.01f);
    limiterReleaseRange.setSkewForCentre (6.0f);
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { LIMITER_RELEASE_ID, versionNum }, LIMITER_RELEASE_NAME, limiterReleaseRange, 300.0f));

    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { LINE_STATE_ID1, versionNum }, LINE_STATE_NAME1, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { LINE_STATE_ID2, versionNum }, LINE_STATE_NAME2, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { LINE_STATE_ID3, versionNum }, LINE_STATE_NAME3, false));

    juce::NormalisableRange<float> freq (40.0f, 10024.0f, 1.0f);
    freq.setSkewForCentre (651.0f);
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { FREQ_ID1, versionNum }, FREQ_NAME1, freq, 21));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { FREQ_ID2, versionNum }, FREQ_NAME2, freq, 21));
    parameters.push_back (std::make_unique<PFloat> (juce::ParameterID { FREQ_ID3, versionNum }, FREQ_NAME3, freq, 21));

    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { BAND_ENABLE_ID1, versionNum }, BAND_ENABLE_NAME1, true));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { BAND_ENABLE_ID2, versionNum }, BAND_ENABLE_NAME2, true));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { BAND_ENABLE_ID3, versionNum }, BAND_ENABLE_NAME3, true));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { BAND_ENABLE_ID4, versionNum }, BAND_ENABLE_NAME4, true));

    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { BAND_SOLO_ID1, versionNum }, BAND_SOLO_NAME1, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { BAND_SOLO_ID2, versionNum }, BAND_SOLO_NAME2, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { BAND_SOLO_ID3, versionNum }, BAND_SOLO_NAME3, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { BAND_SOLO_ID4, versionNum }, BAND_SOLO_NAME4, false));

    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { COMP_BYPASS_ID1, versionNum }, COMP_BYPASS_NAME1, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { COMP_BYPASS_ID2, versionNum }, COMP_BYPASS_NAME2, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { COMP_BYPASS_ID3, versionNum }, COMP_BYPASS_NAME3, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { COMP_BYPASS_ID4, versionNum }, COMP_BYPASS_NAME4, false));

    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { WIDTH_BYPASS_ID1, versionNum }, WIDTH_BYPASS_NAME1, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { WIDTH_BYPASS_ID2, versionNum }, WIDTH_BYPASS_NAME2, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { WIDTH_BYPASS_ID3, versionNum }, WIDTH_BYPASS_NAME3, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { WIDTH_BYPASS_ID4, versionNum }, WIDTH_BYPASS_NAME4, false));

    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { FILTER_BYPASS_ID, versionNum }, FILTER_BYPASS_NAME, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { DOWNSAMPLE_BYPASS_ID, versionNum }, DOWNSAMPLE_BYPASS_NAME, false));
    parameters.push_back (std::make_unique<PBool> (juce::ParameterID { LIMITER_BYPASS_ID, versionNum }, DOWNSAMPLE_BYPASS_NAME, false));
    
    return { parameters.begin(), parameters.end() };
}
