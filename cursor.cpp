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
#include "cursor.h"

Cursor::Cursor(Qt::Orientation orientation, QObject * parent) : QObject::QObject(parent), orientation(orientation)
{

}

int Cursor::fromPoint(QPoint point)
{
    return (orientation == Qt::Vertical) ? point.x() : point.y();
}

bool Cursor::pointOverCursor(QPoint point)
{
    const int margin = 5;
    range_t<int> range = {cursorPosition - margin, cursorPosition + margin};
    return range.contains(fromPoint(point));
}

bool Cursor::eventFilter(QObject *obj, QEvent *event)
{
    // Start dragging on left mouse button press, if over a cursor
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            if (pointOverCursor(mouseEvent->pos())) {
                dragging = true;
                return true;
            }
        }

    // Update current cursor positon if we're dragging
    } else if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (dragging) {
            cursorPosition = fromPoint(mouseEvent->pos());
            emit posChanged();
        }

    // Stop dragging on left mouse button release
    } else if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton && dragging) {
            dragging = false;
            return true;
        }
    }
    return false;
}

int Cursor::pos()
{
    return cursorPosition;
}

void Cursor::setPos(int newPos)
{
    cursorPosition = newPos;
}
