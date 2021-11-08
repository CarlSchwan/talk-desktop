#include "windoweffects.h"

#ifdef HAVE_WINDOWSYSTEM
#include <KWindowEffects>
#include <KWindowSystem>
#endif

#include <QQuickItem>
#include <QQuickWindow>
#include <QDebug>

WindowEffects::WindowEffects(QObject *parent)
    : QObject(parent)
{
}

void WindowEffects::setBlur(QQuickItem *item, bool blur)
{
#ifdef HAVE_WINDOWSYSTEM
    auto setWindows = [item, blur]() {
        auto reg = QRect(QPoint(0, 0), item->window()->size());
        qDebug() << "hello" << item->window() << blur << reg;
        KWindowEffects::enableBackgroundContrast(item->window(), blur, 1, 1, 1, reg);
        KWindowEffects::enableBlurBehind(item->window(), blur, reg);
    };

    disconnect(item->window(), &QQuickWindow::heightChanged, this, nullptr);
    disconnect(item->window(), &QQuickWindow::widthChanged, this, nullptr);
    connect(item->window(), &QQuickWindow::heightChanged, this, setWindows);
    connect(item->window(), &QQuickWindow::widthChanged, this, setWindows);
    setWindows();
#endif
}

void WindowEffects::raiseWindow(QWindow *window)
{
#ifdef HAVE_WINDOWSYSTEM
    KWindowSystem::activateWindow(window->winId());
#endif
}

bool WindowEffects::hasWindowSystem() const
{
#ifdef HAVE_WINDOWSYSTEM
    return true;
#else
    return false;
#endif
}
