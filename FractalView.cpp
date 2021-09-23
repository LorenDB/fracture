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
      m_mandelbrotRect{-2.5, -2, 4, 4, boundingRect()},
      m_juliaRect{-2, -2, 4, 4},
      m_image{boundingRect().size().toSize(), QImage::Format_ARGB32}
{
    connect(this, &FractalView::updateView, this, [this] { update(); }, Qt::QueuedConnection);
}

// the methodology of these functions come from John R. H. Goering's
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
            if (std::abs(zSquaredPlusK) > 2)
                return i + 1;
        }
        return 0;
    }
}

int calculateIsJuliaConnected(const complex &c)
{
    if (std::abs(c) > 2)
        return 1;
    else
    {
        auto zSquaredPlusC = c;
        for (int i = 0; i < 25; ++i)
        {
            zSquaredPlusC = (zSquaredPlusC * zSquaredPlusC) + c;
            if (std::abs(zSquaredPlusC) > 2)
                return i + 1;
        }
        return 0;
    }
}

void FractalView::paint(QPainter *painter)
{
    if (m_mandelbrotRect.visualRect().isEmpty())
        m_mandelbrotRect.setVisualRect(boundingRect());
    if (m_juliaRect.visualRect().isEmpty())
        m_juliaRect.setVisualRect(boundingRect());
    if (m_image.size().isEmpty())
        m_image = QImage{boundingRect().size().toSize(), QImage::Format_ARGB32};

    if (width() != m_width || height() != m_height)
    {
        m_width = width();
        m_height = height();
        m_mandelbrotRect.setVisualRect(boundingRect());
        m_juliaRect.setVisualRect(boundingRect());
        m_image = QImage{boundingRect().size().toSize(), QImage::Format_ARGB32};
        m_isFullyLoaded = false;
    }

    if (!m_isFullyLoaded && !m_isLoading)
    {
        m_isLoading = true;
        emit isLoadingChanged();

        m_image.fill(Qt::transparent);

        auto fut = QtConcurrent::run([this] {
            const auto threads = QThread::idealThreadCount() * 64;
            auto list = (m_type == Type::Julia ? m_juliaRect : m_mandelbrotRect).split(threads);

            m_renderingThreadsMutex.lock();
            m_renderingThreads = threads;
            m_renderingThreadsMutex.unlock();

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
                        if (int result = (m_type == Type::Julia ? calculateJuliaPoint(num, m_juliaPos) : calculateIsJuliaConnected(num)); result == 0)
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

                m_renderingThreadsMutex.lock();
                --m_renderingThreads;
                m_renderingThreadsMutex.unlock();
            });

            m_isFullyLoaded = true;
            m_isLoading = false;
            emit isLoadingChanged();
            emit updateView();
        });

        Q_UNUSED(fut)
    }

    // TODO: this doesn't work for resizes
    m_imageMutex.lock();
    painter->drawImage(boundingRect(), m_image);
    m_imageMutex.unlock();
}

void FractalView::switchToJulia(int x, int y)
{
    cancelRender();

    m_isFullyLoaded = false;
    m_type = Type::Julia;
    emit typeChanged();

    m_juliaPos = m_mandelbrotRect.getFractalValueFromVisualPoint({qreal(x), qreal(y)});
    emit updateView();
}

void FractalView::switchToMandelbrot()
{
    cancelRender();

    m_isFullyLoaded = false;
    m_type = Type::Mandelbrot;
    emit typeChanged();

    emit updateView();
}

void FractalView::cancelRender()
{
    m_cancelRenderRequested = true;
    while (m_renderingThreads > 0)
        qApp->processEvents();
    m_cancelRenderRequested = false;
}

void FractalView::scheduleRender()
{
    if (m_isLoading)
        cancelRender();

    m_isFullyLoaded = false;
    emit updateView();
}

void FractalView::zoomIn()
{
    cancelRender();

    auto &currentRect = getCurrentFractalRect();
    FractalRect newRect{currentRect.x() + currentRect.width() * 0.05,
                       currentRect.y() + currentRect.height() * 0.05,
                       currentRect.width() * 0.9,
                       currentRect.height() * 0.9,
                       currentRect.visualRect()};

    switch (m_type)
    {
    case Type::Mandelbrot:
        m_mandelbrotRect = newRect;
        break;
    case Type::Julia:
        m_juliaRect = newRect;
        break;
    default:
        break;
    }

    scheduleRender();
}

void FractalView::zoomOut()
{
    cancelRender();

    auto &currentRect = getCurrentFractalRect();
    FractalRect newRect{currentRect.x() - currentRect.width() / 0.05,
                       currentRect.y() - currentRect.height() / 0.05,
                       currentRect.width() / 0.9,
                       currentRect.height() / 0.9,
                currentRect.visualRect()};

    switch (m_type)
    {
    case Type::Mandelbrot:
        m_mandelbrotRect = newRect;
        break;
    case Type::Julia:
        m_juliaRect = newRect;
        break;
    default:
        break;
    }

    scheduleRender();
}

FractalRect &FractalView::getCurrentFractalRect()
{
    switch (m_type)
    {
    case Type::Julia:
        return m_juliaRect;
    case Type::Mandelbrot:
    default:
        return m_mandelbrotRect;
    }
}
