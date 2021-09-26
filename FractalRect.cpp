#include "FractalRect.h"

#include <QVector>

FractalRect::FractalRect()
{}

FractalRect::FractalRect(big_float x, big_float y, big_float width, big_float height)
    : m_x{x},
      m_y{y},
      m_width{width},
      m_height{height}
{}

FractalRect::FractalRect(big_float x, big_float y, big_float width, big_float height, QRectF visualRect)
    : m_x{x},
      m_y{y},
      m_width{width},
      m_height{height}
{
    setVisualRect(visualRect);
}

void FractalRect::setVisualRect(const QRectF &visualRect)
{
    if (visualRect.width() / visualRect.height() == m_width / m_height)
        m_visualRect = visualRect;
    else
    {
        // our strategy will be to act like QML's Image.PreserveAspectFit fill mode for Images
        if (visualRect.width() > visualRect.height())
        {
            m_visualRect.setY(visualRect.y());
            m_visualRect.setHeight(visualRect.height());
            m_visualRect.setWidth(m_width.convert_to<qreal>() * visualRect.height() / m_height.convert_to<qreal>());
            m_visualRect.setX(visualRect.x() + (visualRect.width() - m_visualRect.width()) / 2);
        }
        else
        {
            m_visualRect.setX(visualRect.x());
            m_visualRect.setWidth(visualRect.width());
            m_visualRect.setHeight(m_height.convert_to<qreal>() * visualRect.width() / m_width.convert_to<qreal>());
            m_visualRect.setY(visualRect.y() + (visualRect.height() - m_visualRect.height()) / 2);
        }
    }
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
