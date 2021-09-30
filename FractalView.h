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
    Q_PROPERTY(double zoomFactor READ zoomFactor WRITE setZoomFactor RESET resetZoomFactor NOTIFY zoomFactorChanged)
    Q_PROPERTY(double xOffset READ xOffset WRITE setXOffset RESET resetXOffset NOTIFY xOffsetChanged)
    Q_PROPERTY(double yOffset READ yOffset WRITE setYOffset RESET resetYOffset NOTIFY yOffsetChanged)

public:
    enum Type
    {
        Mandelbrot,
        Julia,
        BurningShip,
    };
    Q_ENUM(Type)

    explicit FractalView(QQuickItem *parent = nullptr);
    ~FractalView();

    virtual void paint(QPainter *painter);

    bool isLoading() const { return m_isLoading; }
    Type type() const { return m_type; }
    QPoint juliaPoint() const { return m_juliaPoint; }
    double zoomFactor() const { return m_zoomFactor; }
    double xOffset() const { return m_xOffset; }
    double yOffset() const { return m_yOffset; }

    void setType(Type type);
    void setJuliaPoint(QPoint point);
    void setZoomFactor(double factor);
    void setXOffset(double offset);
    void setYOffset(double offset);

    void resetNavigationRect();
    void resetZoomFactor();
    void resetXOffset();
    void resetYOffset();

signals:
    // since we want to call update() from the render thread(s), this signal is needed to get the call working properly
    void updateView();

    void isLoadingChanged();
    void typeChanged();
    void zoomChanged();
    void juliaPointChanged();
    void zoomFactorChanged();
    void xOffsetChanged();
    void yOffsetChanged();

public slots:
    void cancelRender();
    void rerender();

    void applyZoomIn();
    void applyZoomOut();

    void saveImage(QString filename);

private:
    FractalRect &getCurrentFractalRect();

    QImage m_image;
    bool m_isFullyLoaded{false};
    bool m_isLoading{false};
    Type m_type{Type::Mandelbrot};
    QMap<Type, FractalRect> m_fractalRects;

    // this is a pretty nice default value; let's use it for now
    complex m_juliaPos{0.63982341, 0.123432153};
    QPoint m_juliaPoint;

    // Note: I don't necessarily approve of using qreals;
    // I'm using them here to ensure compatibility with Qt
    qreal m_width{};
    qreal m_height{};

    double m_zoomFactor{1};
    // these offsets are used for navigation
    double m_xOffset{0};
    double m_yOffset{0};

    QMutex m_imageMutex;
    QMutex m_remainingFragmentsMutex;
    int m_remainingFragments;
    bool m_cancelRenderRequested{false};
};

#endif // FRACTALVIEW_H
