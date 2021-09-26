#include "FractalRect.h"

#include <QVector>

FractalRect::FractalRect()
{}

FractalRect::FractalRect(big_float x, big_float y, big_float width, big_float height)
    : m_x{x},
      m_y{y},
      m_width{width},
      m_height{height},
      m_coreX{x},
      m_coreY{y},
      m_coreWidth{width},
      m_coreHeight{height}
{}

FractalRect::FractalRect(big_float x, big_float y, big_float width, big_float height, QRectF visualRect)
    : FractalRect{x, y, width, height}
{
    setVisualRect(visualRect);
}

void FractalRect::setVisualRect(const QRectF &visualRect)
{
    // first we need to parse what our new dimensions will be for the fractal rect
    if (visualRect.width() / visualRect.height() == m_coreWidth / m_coreHeight)
    {
        m_x = m_coreX;
        m_y = m_coreY;
        m_width = m_coreWidth;
        m_height = m_coreHeight;
    }
    else
    {
        if (visualRect.width() > visualRect.height())
        {
            m_width = visualRect.width() * m_coreHeight / visualRect.height();
            m_x = m_coreX - (m_width - m_coreWidth) / 2;
        }
        else
        {
            m_height = visualRect.height() * m_coreWidth / visualRect.width();
            m_y = m_coreY - (m_height - m_coreHeight) / 2;
        }
    }

    m_visualRect = visualRect;
}

QVector<FractalRect> FractalRect::split(int parts)
{
    if (parts == 1)
        return {*this};

    QVector<double> factors; // double to ensure double division later on
    // we'll limit the amount of factors that can be factored out to prevent the application from hanging while trying to factor
    // a massive prime (997 is the last prime before 1000; I was going to put 1000 but it seemed odd to end on a non-prime number)
    for (int i = 2; parts > 1 && i <= 997; ++i)
    {
        while (parts % i == 0)
        {
            factors.push_back(i);
            parts /= i;
        }
    }

    QVector<FractalRect> rects{*this};

    for (const auto &factor : factors)
    {
        for (int i = 0; i < rects.length(); i += factor)
        {
            auto rect = rects[i];
            rects.removeAt(i);
            const auto &vr = rect.m_visualRect;

            // compare visual rect specs because it's probably faster than using a multiprecision number
            if (vr.width() > vr.height())
            {
                for (int j = 0; j < factor; ++j)
                    rects.insert(i + j,
                                 FractalRect{rect.m_x + rect.m_width / factor * j,
                                             rect.m_y,
                                             rect.m_width / factor,
                                             rect.m_height,
                                             QRectF{vr.x() + vr.width() / factor * j,
                                                    vr.y(),
                                                    vr.width() / factor,
                                                    vr.height()}
                                 });
            }
            else
            {
                for (int j = 0; j < factor; ++j)
                    rects.insert(i + j,
                                 FractalRect{rect.m_x,
                                             rect.m_y + rect.m_height / factor * j,
                                             rect.m_width,
                                             rect.m_height / factor,
                                             QRectF{vr.x(),
                                                    vr.y() + vr.height() / factor * j,
                                                    vr.width(),
                                                    vr.height() / factor}
                                 });
            }
        }
    }

    return rects;
}

complex FractalRect::getFractalValueFromVisualPoint(const double &x, const double &y) const
{
    return getFractalValueFromVisualPoint(QPointF{x, y});
}

complex FractalRect::getFractalValueFromVisualPoint(const QPointF &point) const
{
    big_float realOffsetPercent = (point.x() - m_visualRect.x()) / m_visualRect.width();
    big_float imagOffsetPercent = (point.y() - m_visualRect.y()) / m_visualRect.height();

    big_float realPoint = m_width * realOffsetPercent + m_x;
    big_float imagPoint = m_height * imagOffsetPercent + m_y;

    return complex{realPoint, imagPoint};
}
