#include "FractalView.h"

#include <QTimer>
#include <QPainter>
#include <QRandomGenerator64>
#include <QtConcurrent>
#include <QThread>

#include <complex>

FractalView::FractalView(QQuickItem *parent)
    : QQuickPaintedItem{parent},
      m_width{width()},
      m_height{height()},
      m_fractalRects{{Type::Mandelbrot, FractalRect{-2.5, -2, 4, 4}},
                     {Type::Julia, FractalRect{-2, -2, 4, 4}},
                     {Type::BurningShip, FractalRect{-2.5, -2, 4, 4}}},
      m_image{boundingRect().size().toSize(), QImage::Format_ARGB32}
{
    connect(this, &FractalView::updateView, this, [this] { update(); }, Qt::QueuedConnection);
}

FractalView::~FractalView()
{
    cancelRender();
}

// the methodology of these two functions come from John R. H. Goering's
// book `The Powers of the Square Root of -1` and also from
// <https://warp.povusers.org/Mandelbrot>
int calculateJuliaPoint(const complex &z, const complex &k)
{
    if (std::abs(z) > 2)
        return 1;
    else
    {
        auto zSquaredPlusK = z;
        for (int i = 0; i < 50; ++i)
        {
            zSquaredPlusK = (zSquaredPlusK * zSquaredPlusK) + k;
            if (boost::multiprecision::pow(zSquaredPlusK.real(), 2) + boost::multiprecision::pow(zSquaredPlusK.imag(), 2) > 4)
                return i + 1;
        }
        return 0;
    }
}

int calculateMandelbrotPoint(const complex &c)
{
    if (std::abs(c) > 2)
        return 1;
    else
    {
        auto zSquaredPlusC = c;
        for (int i = 0; i < 25; ++i)
        {
            zSquaredPlusC = std::pow(zSquaredPlusC, 2) + c;
            if (boost::multiprecision::pow(zSquaredPlusC.real(), 2) + boost::multiprecision::pow(zSquaredPlusC.imag(), 2) > 4)
                return i + 1;
        }
        return 0;
    }
}

// <https://en.wikipedia.org/wiki/Burning_Ship_fractal> was instrumental in creating this function
int calculateBurningShipPoint(const complex &c)
{
    if (std::abs(c) > 2)
        return 1;
    else
    {
        auto zSquaredPlusC = complex{boost::multiprecision::abs(c.real()), boost::multiprecision::abs(c.imag())};
        for (int i = 0; i < 25; ++i)
        {
            auto temp = std::pow(zSquaredPlusC, 2) + c;
            zSquaredPlusC = complex{boost::multiprecision::abs(temp.real()), boost::multiprecision::abs(temp.imag())};
            if (boost::multiprecision::pow(zSquaredPlusC.real(), 2) + boost::multiprecision::pow(zSquaredPlusC.imag(), 2) > 4)
                return i + 1;
        }
        return 0;
    }
}

void FractalView::paint(QPainter *painter)
{
    for (auto &rect : m_fractalRects)
        if (rect.visualRect().isEmpty())
            rect.setVisualRect(boundingRect());
    if (m_image.size().isEmpty())
        m_image = QImage{boundingRect().size().toSize(), QImage::Format_ARGB32};

    if (width() != m_width || height() != m_height)
    {
        m_width = width();
        m_height = height();
        for (auto &rect : m_fractalRects)
            rect.setVisualRect(boundingRect());
        m_image = QImage{boundingRect().size().toSize(), QImage::Format_ARGB32};
        m_isFullyLoaded = false;
    }

    if (!m_isFullyLoaded && !m_isLoading)
    {
        m_isLoading = true;
        emit isLoadingChanged();

        m_image.fill(Qt::transparent);

        auto fut = QtConcurrent::run([this] {
            const auto fragments = std::min(QThread::idealThreadCount() * 64, static_cast<int>(width() * height()));
            auto list = m_fractalRects[m_type].split(fragments);

            m_remainingFragmentsMutex.lock();
            m_remainingFragments = fragments;
            m_remainingFragmentsMutex.unlock();

            QtConcurrent::blockingMap(list, [this](FractalRect &rect) {
                const auto &vr{rect.visualRect()};

                QImage fragment{vr.size().toSize(), QImage::Format_ARGB32};
                fragment.fill(Qt::transparent);
                QPainter painter;

                auto endX = vr.x() + vr.width();
                auto endY = vr.y() + vr.height();
                if (auto diff = std::abs(endX - static_cast<int>(endX)); diff > 0)
                    endX += (1 - diff);
                if (auto diff = std::abs(endY - static_cast<int>(endY)); diff > 0)
                    endY += (1 - diff);

                for (int i = static_cast<int>(vr.x()); i <= endX; ++i)
                {
                    if (m_cancelRenderRequested)
                        break;

                    painter.begin(&fragment);

                    for (int j = static_cast<int>(vr.y()); j <= endY; ++j)
                    {
                        if (m_cancelRenderRequested)
                            break;

                        if (width() != m_width || height() != m_height)
                            return;

                        complex num = rect.getFractalValueFromVisualPoint(i, j);
                        int result{};
                        switch (m_type)
                        {
                        case Type::Mandelbrot:
                            result = calculateMandelbrotPoint(num);
                            break;
                        case Type::Julia:
                            result = calculateJuliaPoint(num, m_juliaPos);
                            break;
                        case Type::BurningShip:
                            result = calculateBurningShipPoint(num);
                            break;
                        default:
                            break;
                        }

                        if (result == 0)
                            painter.setPen(QPen{QColor{0, 0, 0}});
                        else
                            painter.setPen(QPen{
                                               QColor{255 - std::min(static_cast<int>(255 / result / 0.8) + 50, 255),
                                                      255 - std::min(static_cast<int>(255 / result / 2) + 50, 255),
                                                      255 - std::min(static_cast<int>(255 / result / 4) + 50, 255)}
                                           });
                        painter.drawPoint(i - vr.x(), j - vr.y());
                    }
                    painter.end();

                    m_imageMutex.lock();
                    painter.begin(&m_image);
                    painter.drawImage(vr, fragment);
                    painter.end();
                    m_imageMutex.unlock();
                    emit updateView();
                }

                m_remainingFragmentsMutex.lock();
                --m_remainingFragments;
                m_remainingFragmentsMutex.unlock();
            });

            m_isFullyLoaded = true;
            m_isLoading = false;
            emit isLoadingChanged();
            emit updateView();
        });

        Q_UNUSED(fut)
    }

    m_imageMutex.lock();
    painter->drawImage(boundingRect(), m_image);
    m_imageMutex.unlock();
}

void FractalView::setType(Type type)
{
    if (m_type == type)
        return;

    cancelRender();
    m_type = type;
    m_isFullyLoaded = false;
    emit typeChanged();
    emit updateView();
}

void FractalView::setJuliaPoint(QPoint point)
{
    if (point == m_juliaPoint)
        return;

    m_juliaPoint = point;
    emit juliaPointChanged();

    m_juliaPos = m_fractalRects[Type::Julia].getFractalValueFromVisualPoint(m_juliaPoint);

    if (m_type == Type::Julia)
        rerender();
}

void FractalView::setZoomFactor(double factor)
{
    if (m_zoomFactor == factor || factor > 1 || factor <= 0)
        return;

    m_zoomFactor = factor;
    emit zoomFactorChanged();
}

void FractalView::setXOffset(double offset)
{
    if (m_xOffset == offset)
        return;

    m_xOffset = offset;
    emit xOffsetChanged();
}

void FractalView::setYOffset(double offset)
{
    if (m_yOffset == offset)
        return;

    m_yOffset = offset;
    emit yOffsetChanged();
}

void FractalView::resetZoomFactor()
{
    if (m_zoomFactor == 1)
        return;

    m_zoomFactor = 1;
    emit zoomFactorChanged();
}

void FractalView::resetXOffset()
{
    if (m_xOffset == 0)
        return;

    m_xOffset = 0;
    emit xOffsetChanged();
}

void FractalView::resetYOffset()
{
    if (m_xOffset == 0)
        return;

    m_xOffset = 0;
    emit xOffsetChanged();
}

void FractalView::cancelRender()
{
    m_cancelRenderRequested = true;
    while (m_remainingFragments > 0)
        qApp->processEvents();
    m_cancelRenderRequested = false;
}

void FractalView::rerender()
{
    if (m_isLoading)
        cancelRender();

    m_isFullyLoaded = false;
    emit updateView();
}

void FractalView::applyZoomIn()
{
    if (m_zoomFactor == 1)
        return; // no zoom

    cancelRender();

    auto &currentRect = m_fractalRects[m_type];
    const auto complexValuePerPixel = currentRect.width() / boundingRect().width();
    FractalRect newRect{currentRect.coreX() + (currentRect.coreWidth() - (currentRect.coreWidth() * m_zoomFactor)) / 2 + complexValuePerPixel * m_xOffset,
                       currentRect.coreY() + (currentRect.coreHeight() - (currentRect.coreHeight() * m_zoomFactor)) / 2 + complexValuePerPixel * m_yOffset,
                       currentRect.coreWidth() * m_zoomFactor,
                       currentRect.coreHeight() * m_zoomFactor,
                       currentRect.visualRect()};

    m_fractalRects[m_type] = newRect;

    setZoomFactor(1);
    resetXOffset();
    resetYOffset();

    rerender();
}

void FractalView::applyZoomOut()
{
    if (m_zoomFactor == 1)
        return;

    cancelRender();

    auto &currentRect = m_fractalRects[m_type];
    FractalRect newRect{currentRect.coreX() - ((currentRect.coreWidth() / m_zoomFactor) - currentRect.coreWidth()) / 2,
                       currentRect.coreY() - ((currentRect.coreHeight() / m_zoomFactor) - currentRect.coreHeight()) / 2,
                       currentRect.coreWidth() / m_zoomFactor,
                       currentRect.coreHeight() / m_zoomFactor,
                currentRect.visualRect()};

    m_fractalRects[m_type] = newRect;

    setZoomFactor(1);
    // NOTE: there is not really a good way to apply offsets while zooming out, so we're ignoring them for this function
    resetXOffset();
    resetYOffset();

    rerender();
}
