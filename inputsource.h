#pragma once
#include <fftw3.h>
#include <memory>

class InputSource
{
private:
    FILE *m_file;
    off_t m_file_size;
    fftwf_complex *m_data;
    int m_fft_size;

    fftwf_complex *m_fftw_in = nullptr;
    fftwf_complex *m_fftw_out = nullptr;
    fftwf_plan m_fftw_plan = nullptr;

    std::unique_ptr<float[]> m_window;

    int m_zoom;
    int m_max_zoom;

    int GetFFTStride();
    void cleanupFFTW();
    

public:
    InputSource(const char *filename, int fft_size);
    ~InputSource();

    void GetViewport(float *dest, int x, int y, int width, int height, int zoom);
    int GetHeight();
    int GetWidth();
    void setFFTSize(int size);

    bool ZoomIn();
    bool ZoomOut();
};
