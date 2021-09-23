#ifndef FRACTALRECT_H
#define FRACTALRECT_H

#include <QRectF>

#include "Common.h"

class FractalRect
{
public:
    FractalRect();
    FractalRect(big_float x, big_float y, big_float width, big_float height);
    FractalRect(big_float x, big_float y, big_float width, big_float height, QRectF visualRect);

    void setVisualRect(const QRectF &visualRect);
    QRectF visualRect() const { return m_visualRect; }
    QVector<FractalRect> split(int parts);

    big_float x() const { return m_x; }
    big_float y() const { return m_y; }
    big_float width() const { return m_width; }
    big_float height() const { return m_height; }

    complex getFractalValueFromVisualPoint(const double &x, const double &y) const;
    complex getFractalValueFromVisualPoint(const QPointF &point) const;

private:
    big_float m_x;
    big_float m_y;
    big_float m_width;
    big_float m_height;

    QRectF m_visualRect;
};

#endif // FRACTALRECT_H
