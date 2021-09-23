#ifndef FRACTALVIEW_H
#define FRACTALVIEW_H

#include <QQuickPaintedItem>
#include <QImage>
#include <QMutex>

#include <complex>

#include "Common.h"
#include "FractalRect.h"

class FractalView : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(Type type READ type NOTIFY typeChanged)

public:
    enum Type
    {
        Mandelbrot,
        Julia,
    };
    Q_ENUM(Type)

    explicit FractalView(QQuickItem *parent = nullptr);

    virtual void paint(QPainter *painter);

    bool isLoading() const { return m_isLoading; }
    Type type() const { return m_type; }

signals:
    // since we want to call update() from the render thread(s), this signal is needed to get the call working properly
    void updateView();

    void isLoadingChanged();
    void typeChanged();
    void zoomChanged();

public slots:
    void switchToJulia(int x, int y);
    void switchToMandelbrot();

    void cancelRender();
    void scheduleRender();

    void zoomIn();
    void zoomOut();

private:
    FractalRect &getCurrentFractalRect();

    QImage m_image;
    bool m_isFullyLoaded{false};
    bool m_isLoading{false};
    Type m_type{Type::Mandelbrot};
    FractalRect m_mandelbrotRect;
    FractalRect m_juliaRect;

    complex m_juliaPos;

    // Note: I don't necessarily approve of using qreals;
    // I'm using them here to ensure compatibility with Qt
    qreal m_width{};
    qreal m_height{};

    QMutex m_imageMutex;
    QMutex m_renderingThreadsMutex;
    int m_renderingThreads;
    bool m_cancelRenderRequested{false};
};

#endif // FRACTALVIEW_H
