#include "spectrogram.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QPainter>
#include <QPaintEvent>
#include <QRect>

Spectrogram::Spectrogram()
{
	inputSource = new InputSource("/home/mike/Downloads/hubsan-chopped.cfile", 1024);
	resize(inputSource->GetWidth(), inputSource->GetHeight());
}

Spectrogram::~Spectrogram()
{
	delete inputSource;
}

void Spectrogram::paintEvent(QPaintEvent *event)
{
	QElapsedTimer timer;
	timer.start();

	QRect rect = event->rect();
	QPainter painter(this);
	painter.fillRect(rect, Qt::black);

	if (inputSource != nullptr) {
		int width = inputSource->GetWidth();
		int height = rect.height();

		float *data = (float*)malloc(width * height * sizeof(float));

		inputSource->GetViewport(data, 0, rect.y(), width, height, 0);

		QImage image(width, height, QImage::Format_RGB32);
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				uint8_t pixel = data[y*width + x] * -3;
				image.setPixel(x, y, qRgb(0, 0, pixel));
			}
		}

		QPixmap pixmap = QPixmap::fromImage(image);
		painter.drawPixmap(QRect(0, rect.y(), width, height), pixmap);

		free(data);
	}

	qDebug() << "Paint: " << timer.elapsed() << "ms";
}