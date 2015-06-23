#include "inputsource.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

static const double Tau = M_PI * 2.0;

InputSource::InputSource(const char *filename, int fft_size) {
    m_fft_size = fft_size;

    m_file = fopen(filename, "rb");
    // TODO: throw exception if failed

    struct stat sb;
    fstat(fileno(m_file), &sb);
    // TODO: throw exception if failed
    m_file_size = sb.st_size;

    m_data = (fftwf_complex*)mmap(NULL, m_file_size, PROT_READ, MAP_SHARED, fileno(m_file), 0);
    // TODO: throw exception if failed

    m_fftw_in = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * m_fft_size);
    m_fftw_out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * m_fft_size);
    m_fftw_plan = fftwf_plan_dft_1d(m_fft_size, m_fftw_in, m_fftw_out, FFTW_FORWARD, FFTW_MEASURE);

    m_window.reset(new float[m_fft_size]);
    for (int i = 0; i < m_fft_size; i++) {
        m_window[i] = 0.5f * (1.0f - cos(Tau * i / (m_fft_size - 1)));
    }

    m_zoom = 0;
    m_max_zoom = floor(log2(m_fft_size));
}

InputSource::~InputSource() {
    fftwf_destroy_plan(m_fftw_plan);
    fftwf_free(m_fftw_in);
    fftwf_free(m_fftw_out);

    munmap(m_data, m_file_size);
    fclose(m_file);
}

void InputSource::GetViewport(float *dest, int x, int y, int width, int height, int zoom) {
    fftwf_complex *sample_ptr = &m_data[y * GetOverlap()];

    for (int i = 0; i < height; i++) {
        memcpy(m_fftw_in, sample_ptr, m_fft_size * sizeof(fftw_complex));

        // Apply window
        for (int j = 0; j < m_fft_size; j++) {
            m_fftw_in[j][0] *= m_window[j];
            m_fftw_in[j][1] *= m_window[j];
        }

        fftwf_execute(m_fftw_plan);

        for (int j = x; j < width; j++) {
            int k = (j + m_fft_size / 2) % m_fft_size;
            float re = m_fftw_out[k][0];
            float im = m_fftw_out[k][1];
            float mag = sqrt(re * re + im * im) / m_fft_size;
            float magdb = 10 * log(mag);
            *dest = magdb;
            dest++;
        }
        sample_ptr += GetOverlap();
    }
}

int InputSource::GetHeight() {
    int lines = m_file_size / sizeof(fftwf_complex) / GetOverlap();
    // Force height to be a multiple of overlap size
    return (lines / GetOverlap()) * GetOverlap();
}

int InputSource::GetWidth() {
    return m_fft_size;
}

bool InputSource::ZoomIn() {
    m_zoom++;
    if (m_zoom > m_max_zoom) {
        m_zoom = m_max_zoom;
        return false;
    }
    return true;
}

bool InputSource::ZoomOut() {
    m_zoom--;
    if (m_zoom < 0) {
        m_zoom = 0;
        return false;
    }
    return true;
}

int InputSource::GetOverlap() {
    return m_fft_size / pow(2, m_zoom);
}
