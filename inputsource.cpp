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
    if (m_file == nullptr)
        throw "Error opening file";

    struct stat sb;
    if (fstat(fileno(m_file), &sb) != 0)
        throw "Error fstating file";
    m_file_size = sb.st_size;

    m_data = (fftwf_complex*)mmap(NULL, m_file_size, PROT_READ, MAP_SHARED, fileno(m_file), 0);
    if (m_data == 0)
        throw "Error mmapping file";

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

    free(m_output_cache);
    munmap(m_data, m_file_size);
    fclose(m_file);
}

void InputSource::GetViewport(float *dest, int x, int y, int width, int height, int zoom) {
    int crossover_start_lines = 0;
    int crossover_end_lines = 0;
    if (m_output_cache != nullptr && m_cache_valid) {
        // Scroll down, with at least some of previous cache still in view
        if (m_prev_y <= y && y < m_prev_y + m_prev_height) {
            float *crossover_start = m_output_cache + (y - m_prev_y) * m_fft_size;
            int crossover_lines;
            // This covers the case of a window size reduction
            if (y + height < m_prev_y + m_prev_height) {
                crossover_lines = (m_prev_y + height - y);
            } else {
                crossover_lines = (m_prev_y + m_prev_height - y);
            }
            int crossover_length = crossover_lines * m_fft_size * sizeof(float);
            memcpy(dest, crossover_start, crossover_length);

            crossover_start_lines = crossover_lines;
        // Scroll up
        } else if (y <= m_prev_y && m_prev_y < y + height) {
            float *crossover_start = dest + (m_prev_y - y) * m_fft_size;
            int crossover_lines = y + height - m_prev_y;
            int crossover_length = crossover_lines * m_fft_size * sizeof(float);
            memcpy(crossover_start, m_output_cache, crossover_length);

            crossover_end_lines = crossover_lines;
        }
    }

    if (m_output_cache == nullptr || height != m_prev_height) {
        m_output_cache = (float*)realloc(m_output_cache, m_fft_size * height * sizeof(float));
    }

    m_prev_y = y;
    m_prev_height = height;

    float *cache_ptr = m_output_cache;
    y += crossover_start_lines;
    height -= crossover_start_lines;
    height -= crossover_end_lines;
    dest += crossover_start_lines * m_fft_size;
    cache_ptr += crossover_start_lines * m_fft_size;

    fftwf_complex *sample_ptr = &m_data[y * GetFFTStride()];

    for (int i = 0; i < height; i++) {
        memcpy(m_fftw_in, sample_ptr, m_fft_size * sizeof(fftwf_complex));

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
            float magdb = 10 * log2(mag) / log2(10);
            *dest = magdb;
            dest++;
            *cache_ptr = magdb;
            cache_ptr++;
        }
        sample_ptr += GetFFTStride();
    }
    m_cache_valid = true;
}

int InputSource::GetHeight() {
    int lines = m_file_size / sizeof(fftwf_complex) / GetFFTStride();
    // Force height to be a multiple of overlap size
    return (lines / GetFFTStride()) * GetFFTStride();
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
    m_cache_valid = false;
    return true;
}

bool InputSource::ZoomOut() {
    m_zoom--;
    if (m_zoom < 0) {
        m_zoom = 0;
        return false;
    }
    m_cache_valid = false;
    return true;
}

int InputSource::GetFFTStride() {
    return m_fft_size / pow(2, m_zoom);
}
