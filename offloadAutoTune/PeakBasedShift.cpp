#ifndef PEAKBASEDSHIFT_CPP
#define PEAKBASEDSHIFT_CPP
#include "PeakBasedShift.h"

PeakBasedShift::PeakBasedShift(uint16_t windowSize, uint16_t numberOfWindows, float samplingRate, float overlapPercent, float noiseThresholdInput)
{
    arrSize = windowSize;
    numWindows = numberOfWindows;
    fs = samplingRate;
    overlap = overlapPercent;
    noiseThreshold = noiseThresholdInput;

    calc = new FFT(arrSize);
    previousPhase = new float[arrSize];
    for(int i = 0; i < arrSize; i++)
        previousPhase[i] = 0;
    hop = (1 - overlap) * arrSize;
    currWindow = 0;
    shiftAmount = 0;
}

PeakBasedShift::~PeakBasedShift()
{
    delete [] previousPhase;
    delete calc;
}

void PeakBasedShift::captureWindow(float *input, int shiftVal)
{
    shiftAmount = pow(2, ((float)shiftVal / 12.0));
    for(int i = 0; i < arrSize; i++)
        input[i * 2] = (input[i * 2] - 2048) * .5 * (1 - cos(2*MPI*i/(arrSize-1)));
    calc->calculateFourier(input);
    calc->convertToMagAndPhase(input);
    this->frameCalculation(input);
    calc->convertToComplex(input);
    calc->invertFourier(input);
    currWindow++;
    if(currWindow >= numWindows) 
    {
        currWindow = 0;
        for(int i = 0; i < arrSize; i++)
            previousPhase[i] = 0;
    }
}

void PeakBasedShift::frameCalculation(float *input)
{
    peaks.clear();
    //Captures data
    float *newData = new float[arrSize * 2];
    for(int i = 0; i < (arrSize / 2); i++)
    {
        float currFreq = i * fs / arrSize;
        uint16_t newBin = shiftAmount * currFreq * arrSize / fs;
        float newFreq = newBin * fs / arrSize;
        float freqChange = newFreq - currFreq;
        
        newData[i * 2 + 1] = previousPhase[i] + freqChange * hop;
        newData[i * 2] = 0;
        previousPhase[i] = input[i * 2 + 1];

        newData[(arrSize - i - 1) * 2] = 0;
        newData[(arrSize - i - 1) * 2 + 1] = previousPhase[(arrSize - i - 1)] + freqChange * hop;
    }
    
    //Finds peaks
    for(int i = 2; i < (arrSize / 2) - 2; i++)
    {
        float currValue = input[i * 2];
        if(currValue < noiseThreshold)
            input[i * 2] = 0;
        else if(currValue > input[(i - 2) * 2] && currValue > input[(i - 1) * 2] && currValue > input[(i + 1) * 2] && currValue > input[(i + 2) * 2])
        {
            peaks.push_back(i);
        }
    }
    //Shifts peaks
    uint16_t oldVal = 0;
    for(int i = 0; i < peaks.size(); i++)
    {
        float currFreq = peaks[i] * fs / arrSize;
        uint16_t newBin = shiftAmount * currFreq * arrSize / fs;
        int binShift = newBin - peaks[i];

        int peakVal;
        if(i < peaks.size() - 1)
            peakVal = ceil((peaks[i] + peaks[i + 1]) / 2);
        else
            peakVal = arrSize / 2;
        for(int j = oldVal; j < peakVal; j++)
        {
            if(j + binShift >= 0 && j + binShift < arrSize / 2)
            {
                newData[(j + binShift) * 2] += input[j * 2];
                newData[(arrSize - (j + binShift) - 1) * 2] += input[(arrSize - j - 1) * 2];
            }
        }
        if(i < peaks.size() - 1)
            oldVal = peakVal;
    }
    for(int i = 0; i < arrSize; i++)
    {
        input[i * 2] = newData[i * 2];
        input[i * 2 + 1] = newData[i * 2 + 1];
    }
    delete [] newData;
}

#endif
