#include <QtWidgets>
#include <QRubberBand>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    setWindowTitle(tr("inspectrum"));
    scrollArea.setWidget(&spectrogram);
    scrollArea.viewport()->installEventFilter(this);
    setCentralWidget(&scrollArea);

    dock = new SpectrogramControls(tr("Controls"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    connect(dock, SIGNAL(openFile(QString)), this, SLOT(openFile(QString)));
    connect(dock->sampleRate, SIGNAL(textChanged(QString)), this, SLOT(setSampleRate(QString)));
    connect(dock->centerFreq, SIGNAL(textChanged(QString)), this, SLOT(setCenterFreq(QString)));
    connect(dock, SIGNAL(fftSizeChanged(int)), this, SLOT(setFFTSize(int)));
    connect(dock->zoomLevelSlider, SIGNAL(valueChanged(int)), this, SLOT(setZoomLevel(int)));
    connect(dock->powerMaxSlider, SIGNAL(valueChanged(int)), &spectrogram, SLOT(setPowerMax(int)));
    connect(dock->powerMinSlider, SIGNAL(valueChanged(int)), &spectrogram, SLOT(setPowerMin(int)));
}

QRubberBand *rubberBand=NULL;
QPoint mystart;

bool MainWindow::eventFilter(QObject * /*obj*/, QEvent *event)
{
    if (event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = (QWheelEvent*)event;
        QSlider *slider = nullptr;
        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            slider = dock->zoomLevelSlider;
        } else if (QApplication::keyboardModifiers() & Qt::ShiftModifier) {
            slider = dock->fftSizeSlider;
        }
        if (slider != nullptr) {
            if (wheelEvent->angleDelta().y() > 0) {
                slider->setValue(slider->value() + 1);
            } else if (wheelEvent->angleDelta().y() < 0) {
                slider->setValue(slider->value() - 1);
            }
            return true;
        }
    }else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = (QMouseEvent*)event;
		if (mouseEvent->buttons() == Qt::LeftButton){
			mystart = (mouseEvent->pos());
			if(!rubberBand)
				rubberBand = new QRubberBand(QRubberBand::Rectangle, scrollArea.viewport());
			rubberBand->setGeometry(QRect(mystart, mystart));
			rubberBand->show();
			return true;
		}
    }else if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = (QMouseEvent*)event;
		if (mouseEvent->buttons() == Qt::LeftButton){
			rubberBand->setGeometry(QRect(mystart, mouseEvent->pos()).normalized()); //Area Bounding
			return true;
		}
    }else if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = (QMouseEvent*)event;
		QRect rb= rubberBand->geometry();

		if(rb.top()==rb.bottom() or rb.left()==rb.right() or
			( rb.bottom()-rb.top()<10 and rb.right()-rb.left()<10) ){
			rubberBand->hide();
			rubberBand->clearMask();
			return false;
		}

		int topSample=spectrogram.lineToSample(scrollArea.verticalScrollBar()->value()+ rb.top());
		int bottomSample=spectrogram.lineToSample(scrollArea.verticalScrollBar()->value()+ rb.bottom());

		int leftFreq=(int)(scrollArea.horizontalScrollBar()->value() + rb.left())*spectrogram.getSampleRate()/spectrogram.getFFTSize();
		int rightFreq=(int)(scrollArea.horizontalScrollBar()->value() + rb.right())*spectrogram.getSampleRate()/spectrogram.getFFTSize();

		QStringList command;
		command << "cut_sample" << spectrogram.getFileName() << QString::number(spectrogram.getSampleRate())
		<< QString::number(spectrogram.lineToSample(scrollArea.verticalScrollBar()->value()+ rb.top()))
		<< QString::number(spectrogram.lineToSample(scrollArea.verticalScrollBar()->value()+ rb.bottom()))

		<< QString::number((long int)(scrollArea.horizontalScrollBar()->value() + rb.left())*spectrogram.getSampleRate()/spectrogram.getFFTSize() +spectrogram.getCenterFreq() -spectrogram.getSampleRate()/2)
		<< QString::number((long int)(scrollArea.horizontalScrollBar()->value() + rb.right())*spectrogram.getSampleRate()/spectrogram.getFFTSize() +spectrogram.getCenterFreq() -spectrogram.getSampleRate()/2)
		<< QString::number(spectrogram.getCenterFreq())
		;

		if(system(command.join(" ").toLatin1().data())){
			QMessageBox msgBox;
			msgBox.setText("cut_sample call failed.");
			msgBox.exec();
		};
		rubberBand->hide();
		rubberBand->clearMask();
		return true;
	};
    return false;
}

void MainWindow::setSampleRate(QString rate)
{
    spectrogram.setSampleRate(rate.toInt());
}

void MainWindow::changeSampleRate(int rate)
{
    spectrogram.setSampleRate(rate);
    dock->sampleRate->setText(QString::number(rate));
}

void MainWindow::setCenterFreq(QString rate)
{
    spectrogram.setCenterFreq(rate.toInt());
}

void MainWindow::changeCenterFreq(int rate)
{
    spectrogram.setCenterFreq(rate);
    dock->centerFreq->setText(QString::number(rate));
}

void MainWindow::setFFTSize(int size)
{
    off_t sample = getCenterSample();
    spectrogram.setFFTSize(size);
    scrollArea.verticalScrollBar()->setValue(getScrollPos(sample));
}

void MainWindow::setZoomLevel(int zoom)
{
    off_t sample = getCenterSample();
    spectrogram.setZoomLevel(zoom);
    scrollArea.verticalScrollBar()->setValue(getScrollPos(sample));
}

off_t MainWindow::getCenterSample()
{
    int height = scrollArea.height();
    return (scrollArea.verticalScrollBar()->value() + height / 2) * spectrogram.getStride();
}

int MainWindow::getScrollPos(off_t sample)
{
    int height = scrollArea.height();
    return sample / spectrogram.getStride() - height / 2;
}

void MainWindow::openFile(QString fileName)
{
    QString title="%1: %2";
    this->setWindowTitle(title.arg(QApplication::applicationName(),fileName.section('/',-1,-1)));
    spectrogram.openFile(fileName);
}
