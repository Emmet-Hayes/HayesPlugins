# HayesPlugins

A free, open-source collection of 8 audio plug-ins covering most common use-cases for music and film sound production. Included are a frequency equalizer, a dynamic compressor, a distortion module, a pitch shifter, a simple feedback delay, a tape delay with flutter and wobble, a feedback delay network reverb, and a convolution reverb. Each plug-in is configured to build as VST3 and AU. All written with JUCE.

# HayesCompressor
A free, open-source Compressor audio plug-in.
It provides both input and output gain, a meter to visualize gain reduction, as well as attack, release, threshold, ratio, and knee controls that are very useful in shaping the response of the compressor.

![alt text](https://github.com/Emmet-Hayes/HayesCompressor/raw/master/Images/CompressorGUI.png)


# HayesDelay
A free, open-source feedback delay audio plug-in. 
It creates a circular delay buffer that is written to by the main buffer and then read back from at some time parameter in the past.
With it, users can create longlasting echo chains with feedback, short slapback effects, as well as panned echos.

![alt text](https://github.com/Emmet-Hayes/HayesDelay/raw/master/Images/DelayGUI.png)


# HayesEQ
A free, open-source parametric EQ plugin that features a variable number of EQ bands (1-8) and a fully continuously-resizable GUI.
The number of filters present at any given time can be changed, and each filter can operate as a peak filter, a band-pass, a low-pass or a high-pass.

![alt text](https://github.com/Emmet-Hayes/HayesEQ/raw/master/Images/2xscaleIIRFilterGUI.png)


# HayesFDNReverb
A free, open-source feedback-delay network reverb with 9 all-pass diffusers, 8 delay lines, and some light filtering.
This is a commonly-used technique to create perfect-sounding reverbs, although they can sometimes be tricky to get to sound realistic.

![alt text](https://github.com/Emmet-Hayes/HayesFDNReverb/raw/master/Images/FDNReverbGUI.png)


# HayesOverdrive
A free, open-source audio plugin that can apply various operations to distort the incoming signal.
Users can interact with input and output from the overdrive, giving them control over the amount of overdrive applied to their signal,
as well as the gain of the final result. Also included is a Bias knob, which emulates the characteristics of different types of Diodes in an analog circuit.

![alt text](https://github.com/Emmet-Hayes/HayesOverdrive/raw/master/Images/OverdriveGUI.png)


# HayesPitchShifter
A free, open-source pitch shifter audio plug-in that provides up to one octave up or down, and uses a phase vocoder to achieve the shift. 
It can be run in interval mode, which snaps to equal temperament intervals, or simply tune the signal by some percentage between 50% and 200%.

![alt text](https://github.com/Emmet-Hayes/HayesPitchShifter/raw/master/Images/PitchShifterGUI.png)


# HayesReverb
A free, open-source convolution reverb audio plug-in.
It allows users to control the size, damp-ness, width, and mix of the reverb in their signal, plus a freeze button that perpetuates the 
reverb tail into an infinite loop.

![alt text](https://github.com/Emmet-Hayes/HayesReverb/raw/master/Resources/ReverbGUI.png)


# HayesTapeDelay
A free, open-source feedback delay plug-in with wow and flutter as well as filters added to emulate the characteristics of an analog tape delay.

![alt text](https://github.com/Emmet-Hayes/HayesTapeDelay/raw/master/Images/TapeDelayGUI.png)
