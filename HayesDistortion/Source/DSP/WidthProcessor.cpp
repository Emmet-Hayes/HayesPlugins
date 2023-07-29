#include "WidthProcessor.h"

void WidthProcessor::process(float* channeldataL, float* channeldataR, float width, int numSamples)
{
    // width
    for (int i = 0; i < numSamples; ++i)
    {
        float mid   = (channeldataL[i] + channeldataR[i]) / static_cast<float>(sqrt(2)); // obtain mid-signal from left and right
        float sides = (channeldataL[i] - channeldataR[i]) / static_cast<float>(sqrt(2)); // obtain side-signal from left and right

        // amplify mid and side signal seperately:
        mid *= 2.0f * (1.0f - width);
        sides *= 2.0f * width;

        channeldataL[i] = (mid + sides) / static_cast<float>(sqrt(2)); // obtain left signal from mid and side
        channeldataR[i] = (mid - sides) / static_cast<float>(sqrt(2)); // obtain right signal from mid and side
    }
}

