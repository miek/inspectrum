#pragma once

#include <fftw3.h>

class FFT {
public:
    FFT(int size);
    ~FFT();
    void process(void *dest, void *source);
    int getSize() { return fftSize; }

private:
    int fftSize;
    fftwf_complex *fftwIn = nullptr;
    fftwf_complex *fftwOut = nullptr;
    fftwf_plan fftwPlan = nullptr;
};
