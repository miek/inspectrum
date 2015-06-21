#include <fftw3.h>

#ifndef _INPUTSOURCE_H
#define _INPUTSOURCE_H

class InputSource
{
private:
    FILE *m_file;
    off_t m_file_size;
    fftwf_complex *m_data;
    int m_fft_size;

    fftwf_complex *m_fftw_in;
    fftwf_complex *m_fftw_out;
    fftwf_plan m_fftw_plan;

    float *m_window;
    

public:
    InputSource(const char *filename, int fft_size);
    ~InputSource();

    void GetViewport(float *dest, int x, int y, int width, int height, int zoom);
    int GetHeight();
    int GetWidth();
};

#endif