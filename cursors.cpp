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
#include <QMouseEvent>
#include "cursors.h"

Cursors::Cursors(QObject * parent) : QObject::QObject(parent)
{

}

// Return true if point is over a cursor, put cursor ID in `cursor`
bool Cursors::pointOverCursor(QPoint point, int &cursor)
{
    int margin = 5;
    for (int i = 0; i < 2; i++) {
        range_t<int> range = {cursorPositions[i] - margin, cursorPositions[i] + margin};
        if (range.contains(point.x())) {
            cursor = i;
            return true;
        }
    }
    return false;
}

bool Cursors::eventFilter(QObject *obj, QEvent *event)
{
    // Start dragging on left mouse button press, if over a cursor
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            if (pointOverCursor(mouseEvent->pos(), selectedCursor)) {
                dragging = true;
                return true;
            }
        }

    // Update current cursor positon if we're dragging
    } else if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (dragging) {
            cursorPositions[selectedCursor] = mouseEvent->pos().x();
            emit cursorsMoved();
        }

    // Stop dragging on left mouse button release
    } else if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            dragging = false;
            return true;
        }
    }
    return false;
}

void Cursors::paintFront(QPainter &painter, QRect &rect, range_t<off_t> sampleRange)
{
    painter.save();

    QRect cursorRect(cursorPositions[0], rect.top(), cursorPositions[1] - cursorPositions[0], rect.height());
    // Draw translucent white fill for highlight
    painter.fillRect(
        cursorRect,
        QBrush(QColor(255, 255, 255, 50))
    );

    // Draw vertical edges for individual segments
    painter.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    for (int i = 1; i < segmentCount; i++) {
        int pos = cursorPositions[0] + (i * cursorRect.width() / segmentCount);
        painter.drawLine(pos, rect.top(), pos, rect.bottom());
    }

    // Draw vertical edges
    painter.setPen(QPen(Qt::white, 1, Qt::SolidLine));
    painter.drawLine(cursorPositions[0], rect.top(), cursorPositions[0], rect.bottom());
    painter.drawLine(cursorPositions[1], rect.top(), cursorPositions[1], rect.bottom());

    painter.restore();
}

range_t<int> Cursors::selection()
{
    // TODO: ensure correct ordering during dragging, not here
    if (cursorPositions[0] < cursorPositions[1]) {
        return {cursorPositions[0], cursorPositions[1]};

    } else {
        return {cursorPositions[1], cursorPositions[0]};
    }
}

void Cursors::setSegments(int segments)
{
    segmentCount = std::max(segments, 1);

}

void Cursors::setSelection(range_t<int> selection)
{
    cursorPositions[0] = selection.minimum;
    cursorPositions[1] = selection.maximum;
    emit cursorsMoved();
}
