#include "inputsource.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <stdexcept>

InputSource::InputSource(const char *filename) {
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
}

InputSource::~InputSource() {
    munmap(m_data, m_file_size);
    fclose(m_file);
}

bool InputSource::getSamples(fftwf_complex *dest, off_t start, int length)
{
    if(start < 0 || length < 0)
        return false;

    if (start + length >= sampleCount)
        return false;

    memcpy(dest, &m_data[start], length * sizeof(fftwf_complex));
    return true;
}
