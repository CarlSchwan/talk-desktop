#ifndef AVATARPROVIDER_H
#define AVATARPROVIDER_H

#include <QImage>
#include <QQuickAsyncImageProvider>
#include <QQuickImageResponse>
#include <QSize>
#include <QThreadPool>

class AvatarProvider : public QQuickAsyncImageProvider
{
public:
    AvatarProvider();
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize);

private:
    QThreadPool pool;
};

#endif // AVATARPROVIDER_H
