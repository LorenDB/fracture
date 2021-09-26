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

    big_float coreX() const { return m_coreX; }
    big_float coreY() const { return m_coreY; }
    big_float coreWidth() const { return m_coreWidth; }
    big_float coreHeight() const { return m_coreHeight; }

    complex getFractalValueFromVisualPoint(const double &x, const double &y) const;
    complex getFractalValueFromVisualPoint(const QPointF &point) const;

private:
    // these hold the current size of the rect
    big_float m_x;
    big_float m_y;
    big_float m_width;
    big_float m_height;

    // the core properties hold what was the original rect size and are used to preserve whatever
    // the initial view was if the visual rect changes shape or size (i.e. to prevent zooming in
    // and out due to resize alogorithms)
    big_float m_coreX;
    big_float m_coreY;
    big_float m_coreWidth;
    big_float m_coreHeight;

    QRectF m_visualRect;
};

#endif // FRACTALRECT_H
