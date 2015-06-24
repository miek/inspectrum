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

    float *m_output_cache = nullptr;
    bool m_cache_valid = false;
    int m_prev_y = -1;
    int m_prev_height = -1;

    fftwf_complex *m_fftw_in;
    fftwf_complex *m_fftw_out;
    fftwf_plan m_fftw_plan;

    std::unique_ptr<float[]> m_window;

    int m_zoom;
    int m_max_zoom;

    int GetFFTStride();
    

public:
    InputSource(const char *filename, int fft_size);
    ~InputSource();

    void GetViewport(float *dest, int x, int y, int width, int height, int zoom);
    int GetHeight();
    int GetWidth();

    bool ZoomIn();
    bool ZoomOut();
};
