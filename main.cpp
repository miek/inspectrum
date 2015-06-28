#include <QApplication>
#include <QImage>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <stdio.h>

#include "inputsource.h"

int main(int argc, char *argv[])
{
    const int width = 1024;
    const int height = 256;

    QApplication a(argc, argv);
    InputSource *input = new InputSource(argv[1], width);
    QImage image(width, height, QImage::Format_RGB32);
    float *data = (float*)malloc(width * height * sizeof(float));
    input->GetViewport(data, 0, 0, width, height, 0);

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            *(image.scanLine(y) + x) = (int)data[y * width + x] * -4;
        }
    }

    QLabel lbl;
    lbl.setPixmap(QPixmap::fromImage(image));
    lbl.show();


    return a.exec();

}