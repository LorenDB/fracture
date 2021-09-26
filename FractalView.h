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
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QPoint juliaPoint READ juliaPoint WRITE setJuliaPoint NOTIFY juliaPointChanged)

public:
    enum Type
    {
        Mandelbrot,
        Julia,
        BurningShip,
    };
    Q_ENUM(Type)

    explicit FractalView(QQuickItem *parent = nullptr);

    virtual void paint(QPainter *painter);

    bool isLoading() const { return m_isLoading; }
    Type type() const { return m_type; }
    QPoint juliaPoint() const { return m_juliaPoint; }

    void setType(Type type);
    void setJuliaPoint(QPoint point);

signals:
    // since we want to call update() from the render thread(s), this signal is needed to get the call working properly
    void updateView();

    void isLoadingChanged();
    void typeChanged();
    void zoomChanged();
    void juliaPointChanged();

public slots:
    void cancelRender();
    void scheduleRender();

    void zoomIn();
    void zoomOut();

    void zoomInBy(double factor);
    void zoomOutBy(double factor);

private:
    FractalRect &getCurrentFractalRect();

    QImage m_image;
    bool m_isFullyLoaded{false};
    bool m_isLoading{false};
    Type m_type{Type::Mandelbrot};
    QMap<Type, FractalRect> m_fractalRects;

    complex m_juliaPos;
    QPoint m_juliaPoint;

    // Note: I don't necessarily approve of using qreals;
    // I'm using them here to ensure compatibility with Qt
    qreal m_width{};
    qreal m_height{};

    QMutex m_imageMutex;
    QMutex m_remainingFragmentsMutex;
    int m_remainingFragments;
    bool m_cancelRenderRequested{false};
};

#endif // FRACTALVIEW_H
