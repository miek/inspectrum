#include "spectrogram.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QPainter>
#include <QPaintEvent>
#include <QRect>

Spectrogram::Spectrogram()
{
	sampleRate = 8000000;
	setFFTSize(1024);
	zoomLevel = 0;
	powerMax = 0.0f;
	powerMin = -50.0f;
}

Spectrogram::~Spectrogram()
{
	delete fft;
	delete inputSource;
}

QSize Spectrogram::sizeHint() const {
	return QSize(1024, 2048);
}

void Spectrogram::pickFile()
{
	QString fileName = QFileDialog::getOpenFileName(
		this, tr("Open File"), "", tr("Sample file (*.cfile *.bin);;All files (*.*)")
	);
	if (fileName != nullptr) {
		try {
			InputSource *newFile = new InputSource(fileName.toUtf8().constData());
			delete inputSource;
			inputSource = newFile;
			resize(fftSize, getHeight());
		} catch (std::runtime_error e) {
			// TODO: display error
			return;
		}
	}
}

template <class T> const T& clamp (const T& value, const T& min, const T& max) {
    return std::min(max, std::max(min, value));
}

void HSVtoRGB( float *r, float *g, float *b, float h, float s, float v )
{
    int i;
    float f, p, q, t;

    if( s == 0 ) {
        // achromatic (grey)
        *r = *g = *b = v;
        return;
    }

    h /= 60;            // sector 0 to 5
    i = floor( h );
    f = h - i;          // factorial part of h
    p = v * ( 1 - s );
    q = v * ( 1 - s * f );
    t = v * ( 1 - s * ( 1 - f ) );

    switch( i ) {
        case 0:
            *r = v;
            *g = t;
            *b = p;
            break;
        case 1:
            *r = q;
            *g = v;
            *b = p;
            break;
        case 2:
            *r = p;
            *g = v;
            *b = t;
            break;
        case 3:
            *r = p;
            *g = q;
            *b = v;
            break;
        case 4:
            *r = t;
            *g = p;
            *b = v;
            break;
        default:        // case 5:
            *r = v;
            *g = p;
            *b = q;
            break;
    }
}

void Spectrogram::paintEvent(QPaintEvent *event)
{
	QElapsedTimer timer;
	timer.start();

	QRect rect = event->rect();
	QPainter painter(this);
	painter.fillRect(rect, Qt::black);

	if (inputSource != nullptr) {
		int height = rect.height();

		float *line = (float*)malloc(fftSize * sizeof(float));

		QImage image(fftSize, height, QImage::Format_RGB32);
		for (int y = 0; y < height; y++) {
			getLine(line, rect.y() + y);
			for (int x = 0; x < fftSize; x++) {
				float powerRange = std::abs(powerMin - powerMax);
				float normPower = (line[x] - powerMax) * -1.0f / powerRange;
				normPower = clamp(normPower, 0.0f, 1.0f);

				float red, green, blue;
				HSVtoRGB(&red, &green, &blue, normPower * 300.0f, 1, 1 - normPower);

				image.setPixel(x, y, qRgb(red * 255, green * 255, blue * 255));
			}
		}

		QPixmap pixmap = QPixmap::fromImage(image);
		painter.drawPixmap(QRect(0, rect.y(), fftSize, height), pixmap);

		free(line);
	}

	qDebug() << "Paint: " << timer.elapsed() << "ms";
}

void Spectrogram::getLine(float *dest, int y)
{
	if (inputSource && fft) {
		fftwf_complex buffer[fftSize];
		inputSource->getSamples(buffer, y * getStride(), fftSize);

		for (int i = 0; i < fftSize; i++) {
			buffer[i][0] *= window[i];
			buffer[i][1] *= window[i];
		}

		fft->process(buffer, buffer);
		for (int i = 0; i < fftSize; i++) {
			int k = (i + fftSize / 2) % fftSize;
			float re = buffer[k][0];
			float im = buffer[k][1];
			float mag = sqrt(re * re + im * im) / fftSize;
			float magdb = 10 * log2(mag) / log2(10);
			*dest = magdb;
			dest++;
		}
	}
}

void Spectrogram::setSampleRate(int rate)
{
	sampleRate = rate;
	update();
}

void Spectrogram::setFFTSize(int size)
{
	fftSize = size;
	delete fft;
	fft = new FFT(fftSize);

	window.reset(new float[fftSize]);
	for (int i = 0; i < fftSize; i++) {
		window[i] = 0.5f * (1.0f - cos(Tau * i / (fftSize - 1)));
	}

	resize(fftSize, getHeight());
}

void Spectrogram::setPowerMax(int power)
{
	powerMax = power;
	update();
}

void Spectrogram::setPowerMin(int power)
{
	powerMin = power;
	update();
}

void Spectrogram::setZoomLevel(int zoom)
{
	zoomLevel = clamp(zoom, 0, (int)log2(fftSize));
	resize(fftSize, getHeight());
}

int Spectrogram::getHeight()
{
	if (!inputSource)
		return 0;

	return inputSource->getSampleCount() / getStride();
}

int Spectrogram::getStride()
{
	return fftSize / pow(2, zoomLevel);
}
