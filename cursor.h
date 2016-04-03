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
#include <QPoint>
#include "util.h"

class Cursor : public QObject
{
    Q_OBJECT

public:
    Cursor(Qt::Orientation orientation, QObject * parent);
    int pos();
    void setPos(int newPos);
	bool mouseEvent(QEvent::Type type, QMouseEvent event);

signals:
	void posChanged();

private:
	int fromPoint(QPoint point);
	bool pointOverCursor(QPoint point);

	Qt::Orientation orientation;
	bool dragging = false;
	int cursorPosition = 0;
};
