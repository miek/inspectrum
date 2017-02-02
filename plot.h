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
#include "abstractsamplesource.h"
#include "util.h"

class Plot : public QObject, public Subscriber
{
    Q_OBJECT

public:
    Plot(std::shared_ptr<AbstractSampleSource> src);
    ~Plot();
    void invalidateEvent() override;
    virtual bool mouseEvent(QEvent::Type type, QMouseEvent event);
    virtual std::shared_ptr<AbstractSampleSource> output();
    virtual void paintBack(QPainter &painter, QRect &rect, range_t<off_t> sampleRange);
    virtual void paintMid(QPainter &painter, QRect &rect, range_t<off_t> sampleRange);
    virtual void paintFront(QPainter &painter, QRect &rect, range_t<off_t> sampleRange);
    int height() const { return _height; };

signals:
    void repaint();

protected:
    void setHeight(int height) { _height = height; };

    std::shared_ptr<AbstractSampleSource> sampleSource;

private:
    // TODO: don't hardcode this
    int _height = 200;
};
