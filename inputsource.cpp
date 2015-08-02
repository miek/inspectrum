#include "inputsource.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

static const double Tau = M_PI * 2.0;

InputSource::InputSource(const char *filename, int fft_size) {
    m_file = fopen(filename, "rb");
    if (m_file == nullptr)
        throw std::runtime_error("Error opening file");

    struct stat sb;
    if (fstat(fileno(m_file), &sb) != 0)
        throw std::runtime_error("Error fstating file");
    m_file_size = sb.st_size;
    sampleCount = m_file_size / sizeof(fftwf_complex);

    m_data = (fftwf_complex*)mmap(NULL, m_file_size, PROT_READ, MAP_SHARED, fileno(m_file), 0);
    if (m_data == 0)
        throw std::runtime_error("Error mmapping file");

    setFFTSize(fft_size);
}

InputSource::~InputSource() {
    delete fft;
    munmap(m_data, m_file_size);
    fclose(m_file);
}

bool InputSource::getSamples(fftwf_complex *dest, int start, int length)
{
    if (start + length >= sampleCount)
        return false;

    memcpy(dest, &m_data[start], length * sizeof(fftwf_complex));
    return true;
}

void InputSource::getViewport(float *dest, int x, int y, int width, int height, int zoom) {
    fftwf_complex buffer[m_fft_size];
    fftwf_complex *sample_ptr = &m_data[y * getFFTStride()];

    for (int i = 0; i < height; i++) {
        // Abort if sampling more data than is actually available
        if (sample_ptr > m_data + (m_file_size/sizeof(fftwf_complex)))
            break;

        memcpy(buffer, sample_ptr, m_fft_size * sizeof(fftwf_complex));

        // Apply window
        for (int j = 0; j < m_fft_size; j++) {
            buffer[j][0] *= m_window[j];
            buffer[j][1] *= m_window[j];
        }

        fft->process(buffer, buffer);

        for (int j = x; j < width; j++) {
            int k = (j + m_fft_size / 2) % m_fft_size;
            float re = buffer[k][0];
            float im = buffer[k][1];
            float mag = sqrt(re * re + im * im) / m_fft_size;
            float magdb = 10 * log2(mag) / log2(10);
            *dest = magdb;
            dest++;
        }
        sample_ptr += getFFTStride();
    }
}

int InputSource::getHeight() {
    return m_file_size / sizeof(fftwf_complex) / getFFTStride();
}

int InputSource::getWidth() {
    return m_fft_size;
}

void InputSource::setFFTSize(int size) {
    delete fft;
    fft = new FFT(size);
    m_fft_size = size;

    m_window.reset(new float[m_fft_size]);
    for (int i = 0; i < m_fft_size; i++) {
        m_window[i] = 0.5f * (1.0f - cos(Tau * i / (m_fft_size - 1)));
    }

    m_zoom = 0;
    m_max_zoom = floor(log2(m_fft_size));
}

bool InputSource::zoomIn() {
    m_zoom++;
    if (m_zoom > m_max_zoom) {
        m_zoom = m_max_zoom;
        return false;
    }
    return true;
}

bool InputSource::zoomOut() {
    m_zoom--;
    if (m_zoom < 0) {
        m_zoom = 0;
        return false;
    }
    return true;
}

int InputSource::getFFTStride() {
    return m_fft_size / pow(2, m_zoom);
}
