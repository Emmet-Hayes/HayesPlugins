# HayesPlugins

A collection of 8 audio plug-ins covering most common use-cases for music and film sound production. Included are a frequency equalizer, a dynamic compressor, a distortion module, a pitch shifter, a simple feedback delay, a tape delay with flutter and wobble, a feedback delay network reverb, and a convolution reverb. Each plug-in is configured to build as VST3 and AU.

# HayesCompressor
A Simple Compressor audio plug-in written with JUCE. 
It provides both input and output gain, a meter to visualize gain reduction, as well as all the other bells and whistles you might want for a compressor.

![alt text](https://github.com/Emmet-Hayes/HayesCompressor/raw/master/Images/CompressorGUI.png)


# HayesDelay
A simple feedback delay audio plug-in. 
It creates a circular delay buffer that is written to by the main buffer and then read back from at some time parameter in the past.
With it, users can create longlasting echo chains.

![alt text](https://github.com/Emmet-Hayes/HayesDelay/raw/master/Images/DelayGUI.png)


# HayesEQ
A Basic parametric EQ plugin built with JUCE that features a variable number of filter bands (1-8) and a fully continuously-resizable GUI.
The number of filters present at any given time can be changed, and each filter can operate as a peak filter, a band-pass, a low-pass or a high-pass.

![alt text](https://github.com/Emmet-Hayes/HayesEQ/raw/master/Images/2xscaleIIRFilterGUI.png)


# HayesFDNReverb
A feedback-delay network reverb with 9 all-pass diffusers and 8 delay lines.

This is a commonly-used technique to create perfect-sounding reverbs, although they can be tricky to get to sound realistic.
Future plans for this reverb include parameterizing the diffusion layers as well as the number of delay lines.

![alt text](https://github.com/Emmet-Hayes/HayesFDNReverb/raw/master/Images/FDNReverbGUI.png)


# HayesOverdrive
A simple Overdrive (tanh) plugin utilizing provided constructs within juce::dsp.
Users can interact with input and output from the overdrive, giving them control over the amount of overdrive applied to their signal,
as well as the gain of the final result.

![alt text](https://github.com/Emmet-Hayes/HayesOverdrive/raw/master/Images/OverdriveGUI.png)


# HayesPitchShifter
A simple pitch shifter audio plug-in, provides up to one octave up or down, and uses a phase vocoder to achieve the shift. 
It can also be run in interval mode, which snaps to equal temperament intervals.

![alt text](https://github.com/Emmet-Hayes/HayesPitchShifter/raw/master/Images/PitchShifterGUI.png)


# HayesReverb
A simple reverb audio plug-in written in JUCE. It interacts with the juce::dsp reverb processor provided by the framework.
The user can control the size, the damp-ness, width, and mix of the reverb in their signal, plus a freeze switch for emulating
a sustain stompbox.

![alt text](https://github.com/Emmet-Hayes/HayesReverb/raw/master/Resources/ReverbGUI.png)


# HayesTapeDelay
A digital feedback delay plug-in with wow and flutter as well as filters added to emulate an analog tape delay.

![alt text](https://github.com/Emmet-Hayes/HayesTapeDelay/raw/master/Images/TapeDelayGUI.png)
