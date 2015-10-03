#pragma once

#include "fft.h"
#include <fftw3.h>
#include <memory>

class InputSource
{
private:
    FILE *m_file;
    off_t m_file_size;
    off_t sampleCount;
    fftwf_complex *m_data;

public:
    InputSource(const char *filename);
    ~InputSource();

    bool getSamples(fftwf_complex *dest, off_t start, int length);
    off_t getSampleCount() { return sampleCount; };
};
