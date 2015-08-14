#include <QtWidgets>

#include "mainwindow.h"

MainWindow::MainWindow()
{
    setWindowTitle(tr("inspectrum"));
    scrollArea.setWidget(&spectrogram);
    setCentralWidget(&scrollArea);

    dock = new SpectrogramControls(tr("Controls"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    connect(dock->fileOpenButton, SIGNAL(clicked()), &spectrogram, SLOT(pickFile()));
    connect(dock, SIGNAL(fftSizeChanged(int)), &spectrogram, SLOT(setFFTSize(int)));
    connect(dock->zoomLevelSlider, SIGNAL(valueChanged(int)), this, SLOT(setZoomLevel(int)));
    connect(dock->powerMaxSlider, SIGNAL(valueChanged(int)), &spectrogram, SLOT(setPowerMax(int)));
    connect(dock->powerMinSlider, SIGNAL(valueChanged(int)), &spectrogram, SLOT(setPowerMin(int)));
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
