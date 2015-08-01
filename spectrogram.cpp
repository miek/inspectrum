#include "spectrogram.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QPainter>
#include <QPaintEvent>
#include <QRect>

Spectrogram::Spectrogram()
{
	inputSource = nullptr;
	fftSize = 1024;
	powerMax = 0.0f;
	powerMin = -50.0f;
}

Spectrogram::~Spectrogram()
{
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
			InputSource *newFile = new InputSource(fileName.toUtf8().constData(), fftSize);
			delete inputSource;
			inputSource = newFile;
			resize(inputSource->getWidth(), inputSource->getHeight());
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
		int width = inputSource->getWidth();
		int height = rect.height();

		float *data = (float*)malloc(width * height * sizeof(float));

		inputSource->getViewport(data, 0, rect.y(), width, height, 0);

		QImage image(width, height, QImage::Format_RGB32);
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				float powerRange = std::abs(powerMin - powerMax);
				float normPower = (data[y*width + x] - powerMax) * -1.0f / powerRange;
				normPower = clamp(normPower, 0.0f, 1.0f);

				float red, green, blue;
				HSVtoRGB(&red, &green, &blue, normPower * 300.0f, 1, 1 - normPower);

				image.setPixel(x, y, qRgb(red * 255, green * 255, blue * 255));
			}
		}

		QPixmap pixmap = QPixmap::fromImage(image);
		painter.drawPixmap(QRect(0, rect.y(), width, height), pixmap);

		free(data);
	}

	qDebug() << "Paint: " << timer.elapsed() << "ms";
}

void Spectrogram::setFFTSize(int size)
{
	fftSize = size;
	if (inputSource != nullptr) {
		inputSource->setFFTSize(size);
		update();
		resize(inputSource->getWidth(), inputSource->getHeight());
	}
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