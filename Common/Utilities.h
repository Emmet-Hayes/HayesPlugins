#pragma once
#include <JuceHeader.h>

template<typename FloatType>
static inline juce::NormalisableRange<FloatType> makeLogarithmicRange(FloatType min, FloatType max)
{
    return juce::NormalisableRange<FloatType>
    (
        min, max,
        [] (FloatType start, FloatType end, FloatType normalised)
        {
            return start + (std::pow (FloatType(2), normalised * FloatType(10)) - FloatType(1)) * (end - start) / FloatType(1023);
        },
        [] (FloatType start, FloatType end, FloatType value)
        {
            return (std::log (((value - start) * FloatType(1023) / (end - start)) + FloatType(1)) / std::log (FloatType(2))) / FloatType (10);
        },
        [] (FloatType start, FloatType end, FloatType value)
        {
            if (value > FloatType (3000))
                return juce::jlimit (start, end, FloatType(100) * value / FloatType(100));

            if (value > FloatType(1000))
                return juce::jlimit (start, end, FloatType(10) * value / FloatType(10));

            return juce::jlimit (start, end, FloatType (value));
        }
    );
}