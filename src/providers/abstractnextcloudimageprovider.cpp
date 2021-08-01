#include "abstractnextcloudimageprovider.h"

#include <QException>
#include <QNetworkReply>
#include "../services/accounts.h"

class AsyncImageResponse : public QQuickImageResponse
{
    public:
        AsyncImageResponse(QNetworkRequest request, QThreadPool *pool)
        {
            connect(&m_qnam, &QNetworkAccessManager::finished, this, &AsyncImageResponse::imageReceived);
            m_qnam.get(request);
        }

        void imageReceived(QNetworkReply *reply) {
            m_image.loadFromData(reply->readAll());
            emit finished();
        }

        QQuickTextureFactory *textureFactory() const override
        {
            return QQuickTextureFactory::textureFactoryForImage(m_image);
        }

        QImage m_image;
    private:
        QNetworkAccessManager m_qnam;
        Accounts* m_accountService = Accounts::getInstance();
};

QQuickImageResponse *AbstractNextcloudImageProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    NextcloudAccount* account = account = accountFromId(id);
    if (!account) {
        return nullptr;
    }
    QString subject = id;
    subject.remove(0, id.indexOf('/') + 1);
    QNetworkRequest request = getRequest(subject, account, requestedSize);
    AsyncImageResponse *response = new AsyncImageResponse(request, &pool);
    return response;
}

NextcloudAccount* AbstractNextcloudImageProvider::accountFromId(const QString &id)
{
    const int accountId = id.left(id.indexOf('/')).toInt();
    Accounts* accountService = Accounts::getInstance();
    NextcloudAccount* account = accountService->getAccountById(accountId);
    return account;
}
