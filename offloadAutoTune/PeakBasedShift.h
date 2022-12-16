#ifndef PEAKBASEDSHIFT_H
#define PEAKBASEDSHIFT_H
#include <stdint.h>
#include <math.h>
#include <vector>
#include "FFT.cpp"

class PeakBasedShift
{
    private:
        uint16_t arrSize, numWindows, hop;
        uint8_t currWindow;
        float *previousPhase, fs, overlap, noiseThreshold, shiftAmount;
        std::vector<int> peaks;
        FFT *calc;
        void capturePeaks();
        void shiftPeaks();
        void calculatePhase();
        void frameCalculation(float *input);
    public:
        PeakBasedShift(uint16_t windowSize, uint16_t numberOfWindows, float samplingRate, float overlapPercent, float noiseThresholdInput);
        ~PeakBasedShift();
        void captureWindow(float *input, int shiftVal);
};

#endif