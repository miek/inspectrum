#pragma once
#include <fftw3.h>
#include <memory>

class InputSource
{
private:
    FILE *m_file;
    off_t m_file_size;
    off_t sampleCount;
    fftwf_complex *m_data;
    int m_fft_size;

    fftwf_complex *m_fftw_in = nullptr;
    fftwf_complex *m_fftw_out = nullptr;
    fftwf_plan m_fftw_plan = nullptr;

    std::unique_ptr<float[]> m_window;

    int m_zoom;
    int m_max_zoom;

    int getFFTStride();
    void cleanupFFTW();
    

public:
    InputSource(const char *filename, int fft_size);
    ~InputSource();

    bool getSamples(fftwf_complex *dest, int start, int length);
    void getViewport(float *dest, int x, int y, int width, int height, int zoom);
    int getHeight();
    int getWidth();
    void setFFTSize(int size);

    bool zoomIn();
    bool zoomOut();
};
