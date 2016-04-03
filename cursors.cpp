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

#include <QDebug>
#include "cursors.h"

Cursors::Cursors(QObject * parent) : QObject::QObject(parent)
{
    minCursor = new Cursor(Qt::Vertical, this);
    maxCursor = new Cursor(Qt::Vertical, this);
    connect(minCursor, &Cursor::posChanged, this, &Cursors::cursorMoved);
    connect(maxCursor, &Cursor::posChanged, this, &Cursors::cursorMoved);
}

void Cursors::cursorMoved()
{
    // Swap cursors if one has been dragged past the other
    if (minCursor->pos() > maxCursor->pos()) {
        std::swap(minCursor, maxCursor);
    }
    emit cursorsMoved();
}

bool Cursors::mouseEvent(QEvent::Type type, QMouseEvent event)
{
    if (minCursor->mouseEvent(type, event))
        return true;
    if (maxCursor->mouseEvent(type, event))
        return true;

    return false;
}

void Cursors::paintFront(QPainter &painter, QRect &rect, range_t<off_t> sampleRange)
{
    painter.save();

    QRect cursorRect(minCursor->pos(), rect.top(), maxCursor->pos() - minCursor->pos(), rect.height());

    // Draw translucent white fill for highlight
    painter.fillRect(
        cursorRect,
        QBrush(QColor(255, 255, 255, 50))
    );

    // Draw vertical edges for individual segments
    painter.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    for (int i = 1; i < segmentCount; i++) {
        int pos = minCursor->pos() + (i * cursorRect.width() / segmentCount);
        painter.drawLine(pos, rect.top(), pos, rect.bottom());
    }

    // Draw vertical edges
    painter.setPen(QPen(Qt::white, 1, Qt::SolidLine));
    painter.drawLine(minCursor->pos(), rect.top(), minCursor->pos(), rect.bottom());
    painter.drawLine(maxCursor->pos(), rect.top(), maxCursor->pos(), rect.bottom());

    painter.restore();
}

range_t<int> Cursors::selection()
{
    return {minCursor->pos(), maxCursor->pos()};
}

void Cursors::setSegments(int segments)
{
    segmentCount = std::max(segments, 1);
}

void Cursors::setSelection(range_t<int> selection)
{
    minCursor->setPos(selection.minimum);
    maxCursor->setPos(selection.maximum);
    emit cursorsMoved();
}
