#ifndef ABSTRACTNEXTCLOUDIMAGEPROVIDER_H
#define ABSTRACTNEXTCLOUDIMAGEPROVIDER_H

#include <QImage>
#include <QNetworkRequest>
#include <QQuickAsyncImageProvider>
#include <QQuickImageResponse>
#include <QSize>
#include <QThreadPool>
#include "../services/accounts.h"

class AbstractNextcloudImageProvider : public QQuickAsyncImageProvider
{
public:
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize);
    virtual QNetworkRequest getRequest(QString subject, NextcloudAccount account, const QSize &requestedSize) = 0;

private:
    NextcloudAccount accountFromId(const QString &id);
    QThreadPool pool;
};

#endif // ABSTRACTNEXTCLOUDIMAGEPROVIDER_H
