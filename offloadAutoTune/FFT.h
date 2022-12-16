#ifndef FFT_H
#define FFT_H
#include <stdint.h>
#include <math.h>

const double MPI = 3.141592653589793238460;

class FFT{
    private:
        uint16_t arrSize, layers;
        float **twiddleFactors;
        
        void fft(float *, int direction);
        void buildTwiddle();
        uint16_t reverseBitOrder(uint16_t x);
        float findMagnitude(float a, float b);
        float findPhase(float a, float b);
    public:
        FFT();
        FFT(uint16_t n);
        ~FFT();
        void calculateFourier(float *);
        void invertFourier(float *);
        void convertToMagAndPhase(float *);
        void convertToComplex(float *);
};

#endif