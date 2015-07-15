#pragma once

#include <QMainWindow>
#include <QScrollArea>
#include "spectrogram.h"
#include "spectrogramcontrols.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private:
    QScrollArea scrollArea;
    Spectrogram spectrogram;
    SpectrogramControls *dock;
};