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

Tuner::Tuner(QObject * parent) : QObject::QObject(parent)
{
    minCursor = new Cursor(Qt::Horizontal, this);
    cfCursor = new Cursor(Qt::Horizontal, this);
    maxCursor = new Cursor(Qt::Horizontal, this);
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
        _deviation = std::max(2, std::abs(sender->pos() - cfCursor->pos()));
    }

    updateCursors();
}

int Tuner::deviation()
{
    return _deviation;
}

bool Tuner::eventFilter(QObject *obj, QEvent *event)
{
    if (minCursor->eventFilter(obj, event))
        return true;
    if (cfCursor->eventFilter(obj, event))
        return true;
    if (maxCursor->eventFilter(obj, event))
        return true;

    return false;
}

void Tuner::paintFront(QPainter &painter, QRect &rect, range_t<off_t> sampleRange)
{
    painter.save();

    QRect cursorRect(rect.left(), minCursor->pos(), rect.right(), maxCursor->pos() - minCursor->pos());

    // Draw translucent white fill for highlight
    painter.fillRect(
        cursorRect,
        QBrush(QColor(255, 255, 255, 50))
    );

    // Draw tuner edges
    painter.setPen(QPen(Qt::white, 1, Qt::SolidLine));
    painter.drawLine(rect.left(), minCursor->pos(), rect.right(), minCursor->pos());
    painter.drawLine(rect.left(), maxCursor->pos(), rect.right(), maxCursor->pos());

    // Draw centre freq
    painter.setPen(QPen(Qt::red, 1, Qt::SolidLine));
    painter.drawLine(rect.left(), cfCursor->pos(), rect.right(), cfCursor->pos());

    painter.restore();
}

void Tuner::setCentre(int centre)
{
    cfCursor->setPos(centre);
    updateCursors();
}

void Tuner::setDeviation(int dev)
{
    _deviation = dev;
    updateCursors();
}

void Tuner::updateCursors()
{
    minCursor->setPos(cfCursor->pos() - _deviation);
    maxCursor->setPos(cfCursor->pos() + _deviation);
    emit tunerMoved();
}
