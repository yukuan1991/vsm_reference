#pragma once

#include <QPointF>
#include <QPolygonF>
#include <QLineF>
#include <QMatrix>

inline qreal distance (QPointF p1, QPointF p2)
{
    auto x1 = p1.x(), x2 = p2.x(), y1 = p1.y(), y2 = p2.y();

    auto x_diff = x2 - x1, y_diff = y2 - y1;

    return sqrt(x_diff * x_diff + y_diff * y_diff);
}


inline QPolygonF get_line_polygon (QLineF l, qreal width)
{
    const auto mid_pos = l.pointAt (0.5);
    const auto len = l.length ();
    QMatrix matrix;

    matrix.translate (mid_pos.x (), mid_pos.y ());
    matrix.rotate (- l.angle ());

    const auto y_diff = width / 2;
    const auto rect = QRectF (- len / 2, - y_diff, len, width);
    const auto polygon = QPolygonF (rect);
    auto mapped_polygon = matrix.map (polygon);

    return mapped_polygon;
}
