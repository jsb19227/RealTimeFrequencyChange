#ifndef FFT_CPP
#define FFT_CPP

#include "FFT.h"

FFT::FFT()
{
    arrSize = 64;
    layers = log(arrSize) / log(2);
    twiddleFactors = new float*[(1 << (layers - 1))];
    for(int i = 0; i < (1 << (layers - 1)); i++)
        twiddleFactors[i] = new float[2];
    buildTwiddle();
}

FFT::FFT(uint16_t n)
{
    arrSize = n;
    layers = log(arrSize) / log(2);
    twiddleFactors = new float*[(1 << (layers - 1))];
    for(int i = 0; i < (1 << (layers - 1)); i++)
        twiddleFactors[i] = new float[2];
    buildTwiddle();
}

FFT::~FFT()
{
    for(int i = 0; i < (1 << (layers - 1)); i++)
    {
        delete [] twiddleFactors[i];
    }
    delete [] twiddleFactors;
}

void FFT::buildTwiddle()
{
    twiddleFactors[0][0] = 1;
    twiddleFactors[0][1] = 0;
    
    for(int i = 1; i < (1 << (layers - 1)); i++)
    {
        twiddleFactors[i][0] = cos(i * -2 * MPI / arrSize);
        twiddleFactors[i][1] = sin(i * -2 * MPI / arrSize);
    }

}

void FFT::calculateFourier(float *input)
{
    fft(input, 1);
}

void FFT::invertFourier(float *input)
{
    fft(input, -1);
    for(int i = 0; i < arrSize; i++)
        input[i * 2] /= arrSize;
}

uint16_t FFT::reverseBitOrder(uint16_t x)
{
    uint16_t bits = log(arrSize) / log(2);
    uint16_t output = 0;
    for(int i = 0; i < bits; i++)
    {
        output |= ((x >> i) & 1) << (bits - i - 1);
    }
    return output;
}

float FFT::findMagnitude(float a, float b)
{
    return pow(pow(a, 2) + pow(b, 2), .5);
}

float FFT::findPhase(float a, float b)
{
    if(a == 0 && b == 0)
        return 0;
    if(a == 0 && b < 0)
        return MPI / -2;
    if(a == 0 && b > 0)
        return MPI / 2;
    if(a < 0 && b < 0)
        return atan(b / a) - MPI;
    if(a < 0 && b >= 0)
        return atan(b / a) + MPI;
    return atan(b / a);
}

void FFT::convertToMagAndPhase(float *input)
{
    for(int i = 0; i < arrSize; i++)
    {
        float mag = findMagnitude(input[i * 2], input[i * 2 + 1]);
        float ang = findPhase(input[i * 2], input[i * 2 + 1]);
        input[i * 2] = mag;
        input[i * 2 + 1] = ang;
    }
}

void FFT::convertToComplex(float *input)
{
    for(int i = 0; i < arrSize; i++)
    {
        float a = input[i * 2] * cos(input[i * 2 + 1]);
        float b = input[i * 2] * sin(input[i * 2 + 1]);
        input[i * 2] = a;
        input[i * 2 + 1] = b;
    }
}

void FFT::fft(float *fourier, int direction)
{
    for(int i = 0; i < arrSize; i++)
    {
        uint16_t index = reverseBitOrder(i);
        if(index > i)
        {
            float tempReal = fourier[i * 2];
            float tempFake = fourier[i * 2 + 1];
            fourier[i * 2] = fourier[index * 2];
            fourier[i * 2 + 1] = fourier[index * 2 + 1];
            fourier[index * 2] = tempReal;
            fourier[index * 2 + 1] = tempFake;
        }
    }
    for(int i = 1; i <= layers; i++)
    {
        int j = 0, count = 1, modifier = (1 << (layers - i));
        while(j < arrSize)
        {    
            int index = j;
            int companion = j + (1 << (i - 1));

            int power = (count - 1) * modifier;

            float tempReal = twiddleFactors[power][0] * fourier[2 * companion] - direction * twiddleFactors[power][1] * fourier[2 * companion + 1];
            float tempIm = twiddleFactors[power][0] * fourier[2 * companion + 1] + direction * twiddleFactors[power][1] * fourier[2 * companion];

            fourier[2 * companion] = fourier[2 * index] - tempReal;
            fourier[2 * companion + 1] = fourier[2 * index + 1] - tempIm;
            fourier[2 * index] = fourier[2 * index] + tempReal;
            fourier[2 * index + 1] = fourier[2 * index + 1] + tempIm;

            if(count == (1 << (i - 1))) 
            {
                j += (1 << (i - 1)) + 1;
                count = 1;
            }
            else 
            {
                j++;
                count++;
            }
        }
    }
}

#endif