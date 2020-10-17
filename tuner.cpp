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

#include <cstdlib>
#include "tuner.h"

Tuner::Tuner(int height, QObject * parent) : height(height), QObject::QObject(parent)
{
    minCursor = new Cursor(Qt::Horizontal, Qt::SizeVerCursor, this);
    cfCursor = new Cursor(Qt::Horizontal, Qt::SizeAllCursor, this);
    maxCursor = new Cursor(Qt::Horizontal, Qt::SizeVerCursor, this);
    connect(minCursor, &Cursor::posChanged, this, &Tuner::cursorMoved);
    connect(cfCursor, &Cursor::posChanged, this, &Tuner::cursorMoved);
    connect(maxCursor, &Cursor::posChanged, this, &Tuner::cursorMoved);

    cfCursor->setPos(100);
    _deviation = 10;
    updateCursors();
}

int Tuner::centre()
{
    return cfCursor->pos();
}

void Tuner::cursorMoved()
{
    Cursor *sender = static_cast<Cursor*>(QObject::sender());
    if (sender != cfCursor) {
        // Limit cursor positions to within plot
        auto posRange = range_t<int>{0, height};
        sender->setPos(posRange.clip(sender->pos()));

        // Limit deviation range to half of total BW (either side of centre)
        auto deviationRange = range_t<int>{2, height / 2};
        _deviation = deviationRange.clip(std::abs(sender->pos() - cfCursor->pos()));
    } else {
        auto cfRange = range_t<int>{_deviation, height - _deviation};
        sender->setPos(cfRange.clip(sender->pos()));
    }

    updateCursors();
}

int Tuner::deviation()
{
    return _deviation;
}

bool Tuner::mouseEvent(QEvent::Type type, QMouseEvent event)
{
    if (cfCursor->mouseEvent(type, event))
        return true;
    if (minCursor->mouseEvent(type, event))
        return true;
    if (maxCursor->mouseEvent(type, event))
        return true;

    return false;
}

void Tuner::paintFront(QPainter &painter, QRect &rect, range_t<size_t> sampleRange)
{
    painter.save();

    QRect cursorRect(rect.left(), rect.top() + minCursor->pos(), rect.right(), maxCursor->pos() - minCursor->pos());

    // Draw translucent white fill for highlight
    painter.fillRect(
        cursorRect,
        QBrush(QColor(255, 255, 255, 50))
    );

    // Draw tuner edges
    painter.setPen(QPen(Qt::white, 1, Qt::SolidLine));
    painter.drawLine(rect.left(), rect.top() + minCursor->pos(), rect.right(), rect.top() + minCursor->pos());
    painter.drawLine(rect.left(), rect.top() + maxCursor->pos(), rect.right(), rect.top() + maxCursor->pos());

    // Draw centre freq
    painter.setPen(QPen(Qt::red, 1, Qt::SolidLine));
    painter.drawLine(rect.left(), rect.top() + cfCursor->pos(), rect.right(), rect.top() + cfCursor->pos());

    painter.restore();
}

void Tuner::setCentre(int centre)
{
    cfCursor->setPos(centre);
    updateCursors();
}

void Tuner::setDeviation(int dev)
{
    _deviation = std::max(1, dev);
    updateCursors();
    emit tunerMoved(_deviation);
}

void Tuner::setHeight(int height)
{
    this->height = height;
}

void Tuner::updateCursors()
{
    minCursor->setPos(cfCursor->pos() - _deviation);
    maxCursor->setPos(cfCursor->pos() + _deviation);
    emit tunerMoved(_deviation);
}
