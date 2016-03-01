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

#include <QObject>
#include <QPainter>
#include "util.h"

class Plot : public QObject
{

public:
    virtual void paintBack(QPainter &painter, QRect &rect, range_t<off_t> sampleRange) = 0;
    virtual void paintMid(QPainter &painter, QRect &rect, range_t<off_t> sampleRange) = 0;
    virtual void paintFront(QPainter &painter, QRect &rect, range_t<off_t> sampleRange) = 0;
    int height() const { return _height; };

protected:
    void setHeight(int height) { _height = height; };

private:
    int _height = 50;
};
