#pragma once

#include <QPointF>

inline qreal distance (QPointF p1, QPointF p2)
{
    auto x1 = p1.x(), x2 = p2.x(), y1 = p1.y(), y2 = p2.y();

    auto x_diff = x2 - x1, y_diff = y2 - y1;

    return sqrt(x_diff * x_diff + y_diff * y_diff);
}
