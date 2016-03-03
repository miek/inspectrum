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

#include <QPainter>
#include "cursors.h"

Cursors::Cursors(QWidget * parent)
    : QWidget::QWidget(parent)
{

}

void Cursors::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.save();

    // Draw translucent white fill for highlight
    painter.fillRect(rect(), QBrush(QColor(255, 255, 255, 50)));

    // Draw vertical edges
    QPen pen(Qt::white, 1, Qt::DashLine);
    painter.setPen(pen);
    painter.drawLine(rect().left(), rect().top(), rect().left(), rect().bottom());
    painter.drawLine(rect().right(), rect().top(), rect().right(), rect().bottom());

    painter.restore();
}
