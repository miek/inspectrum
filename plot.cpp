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

#include "plot.h"

bool Plot::mouseEvent(QEvent::Type type, QMouseEvent event)
{

}

void Plot::paintBack(QPainter &painter, QRect &rect, range_t<off_t> sampleRange)
{
    painter.save();
    QPen pen(Qt::white, 1, Qt::DashLine);
    painter.setPen(pen);
    painter.drawLine(rect.left(), rect.center().y(), rect.right(), rect.center().y());
    painter.restore();
}

void Plot::paintMid(QPainter &painter, QRect &rect, range_t<off_t> sampleRange)
{

}

void Plot::paintFront(QPainter &painter, QRect &rect, range_t<off_t> sampleRange)
{

}
