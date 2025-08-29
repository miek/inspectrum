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

#include <QApplication>
#include <QDebug>
#include "cursors.h"

Cursors::Cursors(QObject * parent) : QObject::QObject(parent)
{
    minCursor = new Cursor(Qt::Vertical, Qt::SizeHorCursor, this);
    maxCursor = new Cursor(Qt::Vertical, Qt::SizeHorCursor, this);
    connect(minCursor, &Cursor::posChanged, this, &Cursors::cursorMoved);
    connect(maxCursor, &Cursor::posChanged, this, &Cursors::cursorMoved);
}

void Cursors::frozen(bool enable) {
	minCursor->frozen(enable);
	maxCursor->frozen(enable);
}

void Cursors::cursorMoved()
{
    // Swap cursors if one has been dragged past the other
    if (minCursor->pos() > maxCursor->pos()) {
        std::swap(minCursor, maxCursor);
    }
    emit cursorsMoved();
}

bool Cursors::pointWithinDragRegion(QPoint point) {
    int margin = 10;
    range_t<int> range = {minCursor->pos()+margin, maxCursor->pos()-margin};
    return range.contains(point.x());
}

bool Cursors::mouseEvent(QEvent::Type type, QMouseEvent event)
{
    if (minCursor->mouseEvent(type, event))
        return true;
    if (maxCursor->mouseEvent(type, event))
    return true;

    // If the mouse pointer is between the cursors, display a resize pointer
    if (pointWithinDragRegion(event.pos()) && type != QEvent::Leave) {
        if (!cursorOverride) {
                cursorOverride = true;
                QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
        }
    // Restore pointer otherwise
    } else {
        if (cursorOverride) {
            cursorOverride = false;
            QApplication::restoreOverrideCursor();
        }
    }
    // Start dragging on left mouse button press, if between the cursors
    if (type == QEvent::MouseButtonPress) {
        if (event.button() == Qt::LeftButton) {
            if (pointWithinDragRegion(event.pos())) {
                dragging = true;
                dragPos = event.pos();
                return true;
            }
        }
    // Update both cursor positons if we're dragging
    } else if (type == QEvent::MouseMove) {
        if (dragging) {
            int dx = event.pos().x() - dragPos.x();
            minCursor->setPos(minCursor->pos() + dx);
            maxCursor->setPos(maxCursor->pos() + dx);
            dragPos = event.pos();
            emit cursorsMoved();
        }
    // Stop dragging on left mouse button release
    } else if (type == QEvent::MouseButtonRelease) {
        if (event.button() == Qt::LeftButton && dragging) {
            dragging = false;
            return true;
        }
    }
    return false;
}

void Cursors::paintFront(QPainter &painter, QRect &rect, range_t<size_t> sampleRange)
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
    for (long i = 1; i < segmentCount; i++) {
        int pos = minCursor->pos() + (i * cursorRect.width() / segmentCount);
        painter.drawLine(pos, rect.top(), pos, rect.bottom());
    }

    // Draw vertical edges
    painter.setPen(QPen(Qt::white, 1, Qt::SolidLine));
    painter.drawLine(minCursor->pos(), rect.top(), minCursor->pos(), rect.bottom());
    painter.drawLine(maxCursor->pos(), rect.top(), maxCursor->pos(), rect.bottom());

    painter.restore();
}

int Cursors::segments()
{
    return segmentCount;
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
}
