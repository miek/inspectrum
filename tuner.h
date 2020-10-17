/*
 *  Copyright (C) 2016, Mike Walters <mike@flomp.net>
 *
 *  This file is part of inspectrum.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QPoint>
#include "cursor.h"
#include "util.h"

class Tuner : public QObject
{
    Q_OBJECT

public:
    Tuner(int height, QObject * parent);
    int centre();
    int deviation();
    bool mouseEvent(QEvent::Type, QMouseEvent event);
    void paintFront(QPainter &painter, QRect &rect, range_t<size_t> sampleRange);
    void setCentre(int centre);
    void setDeviation(int dev);
    void setHeight(int height);

public slots:
    void cursorMoved();

signals:
    void tunerMoved(int deviation);

private:
    void updateCursors();

    Cursor *minCursor;
    Cursor *cfCursor;
    Cursor *maxCursor;
    int _deviation;
    int height;
};
