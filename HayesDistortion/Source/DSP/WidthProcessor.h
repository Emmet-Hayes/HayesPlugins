#pragma once

#include <JuceHeader.h>


class WidthProcessor
{
public:
    WidthProcessor() {}
    void process(float* channeldataL, float* channeldataR, float width, int numSamples);
};
