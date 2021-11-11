#include "abstractnextcloudimageprovider.h"

#include <QException>
#include <QNetworkReply>
#include "../services/accounts.h"

#include <QAtomicPointer>
#include <QReadWriteLock>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

class AsyncImageResponse : public QQuickImageResponse
{
    public:
        AsyncImageResponse(QNetworkRequest request, QThreadPool *pool, const QString &id = QString(), QSize size = {})
            : m_accountService(AccountModel::getInstance())
        {
            const QString localFile = QStringLiteral("%1/image_provider/%2-%3x%4.png")
                .arg(QStandardPaths::writableLocation(QStandardPaths::CacheLocation), id,
                             QString::number(size.width()),
                             QString::number(size.height()));

            QImage cachedImage;
            if (cachedImage.load(localFile)) {
                m_image = cachedImage;
                Q_EMIT finished();
                return;
            }

            auto reply = m_qnam.get(request);
            connect(reply, &QNetworkReply::finished, this, [this, localFile, reply]() {
                QWriteLocker _(&lock);
                m_image.loadFromData(reply->readAll());

                QString localPath = QFileInfo(localFile).absolutePath();
                QDir dir;
                if (!dir.exists(localPath)) {
                    dir.mkpath(localPath);
                }

                m_image.save(localFile);

                Q_EMIT finished();
            });
        }

        QQuickTextureFactory *textureFactory() const override
        {
            return QQuickTextureFactory::textureFactoryForImage(m_image);
        }

    private:
        QImage m_image;
        mutable QReadWriteLock lock; // Guards ONLY the member above
        QNetworkAccessManager m_qnam;
        AccountModel *m_accountService = nullptr;
};

QQuickImageResponse *AbstractNextcloudImageProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    auto account = accountFromId(id);
    if (!account) {
        return nullptr;
    }
    QString subject = id;
    subject.remove(0, id.indexOf('/') + 1);
    QNetworkRequest request = getRequest(subject, account, requestedSize);
    AsyncImageResponse *response = new AsyncImageResponse(request, &pool, id, requestedSize);
    return response;
}

NextcloudAccount* AbstractNextcloudImageProvider::accountFromId(const QString &id)
{
    const int accountId = id.left(id.indexOf('/')).toInt();
    const auto accountService = AccountModel::getInstance();
    const auto account = accountService->getAccountById(accountId);
    if (!account) {
        qDebug() << "No account found for id" << id << accountService->getAccounts();
    }
    return account;
}
