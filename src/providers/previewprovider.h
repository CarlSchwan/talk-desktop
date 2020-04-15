#ifndef PREVIEWPROVIDER_H
#define PREVIEWPROVIDER_H

#include <QQuickAsyncImageProvider>
#include <QQuickImageResponse>
#include <QSize>
#include <QThreadPool>

class PreviewProvider : public QQuickAsyncImageProvider
{
public:
    PreviewProvider();
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize);

private:
    QThreadPool pool;
};

#endif // PREVIEWPROVIDER_H
