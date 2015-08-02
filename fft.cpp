#include "fft.h"
#include "string.h"

FFT::FFT(int size)
{
    fftSize = size;

    fftwIn = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * fftSize);
    fftwOut = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * fftSize);
    fftwPlan = fftwf_plan_dft_1d(fftSize, fftwIn, fftwOut, FFTW_FORWARD, FFTW_MEASURE);
}

FFT::~FFT()
{
    if (fftwPlan) fftwf_destroy_plan(fftwPlan);
    if (fftwIn) fftwf_free(fftwIn);
    if (fftwOut) fftwf_free(fftwOut);
}

void FFT::process(void *dest, void *source)
{
    memcpy(fftwIn, source, fftSize * sizeof(fftwf_complex));
    fftwf_execute(fftwPlan);
    memcpy(dest, fftwOut, fftSize * sizeof(fftwf_complex));
}
