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

class Cursors : public QObject
{
    Q_OBJECT

public:
    Cursors(QObject * parent);
    bool mouseEvent(QEvent::Type type, QMouseEvent event);
    void paintFront(QPainter &painter, QRect &rect, range_t<off_t> sampleRange);
    range_t<int> selection();
    void setSegments(int segments);
    void setSelection(range_t<int> selection);

public slots:
	void cursorMoved();

signals:
	void cursorsMoved();


private:
	bool pointOverCursor(QPoint point, int &cursor);

	Cursor *minCursor;
	Cursor *maxCursor;
	int segmentCount = 1;
};
