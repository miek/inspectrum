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
#include "cursor.h"

Cursor::Cursor(Qt::Orientation orientation, Qt::CursorShape mouseCursorShape, QObject * parent) : QObject::QObject(parent), orientation(orientation), cursorShape(mouseCursorShape)
{

}

void Cursor::frozen(bool enable) {
	isFrozen = enable;
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

bool Cursor::mouseEvent(QEvent::Type type, QMouseEvent event)
{
	if (isFrozen) {
		return false;
	}
    // If the mouse pointer moves over a cursor, display a resize pointer
	if (pointOverCursor(event.pos()) && type != QEvent::Leave) {
		if (!cursorOverrided) {
			cursorOverrided = true;
			QApplication::setOverrideCursor(QCursor(cursorShape));
		}

	// Restore pointer if it moves off the cursor, or leaves the widget
	} else if (cursorOverrided) {
		cursorOverrided = false;
		QApplication::restoreOverrideCursor();
	}


    // Start dragging on left mouse button press, if over a cursor
    if (type == QEvent::MouseButtonPress) {
        if (event.button() == Qt::LeftButton) {
            if (pointOverCursor(event.pos())) {
                dragging = true;
                return true;
            }
        }

    // Update current cursor position if we're dragging
    } else if (type == QEvent::MouseMove) {
        if (dragging) {
            cursorPosition = fromPoint(event.pos());
            emit posChanged();
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

int Cursor::pos()
{
    return cursorPosition;
}

void Cursor::setPos(int newPos)
{
    cursorPosition = newPos;
}
