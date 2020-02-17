#include "avatarprovider.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include "../services/accounts.h"

AvatarProvider::AvatarProvider() : QQuickAsyncImageProvider()
{

}

class AsyncAvatarResponse : public QQuickImageResponse
{
    public:
        AsyncAvatarResponse(const QString &id, const QSize &requestedSize, QThreadPool *pool)
        {
            const int accountId = id.left(id.indexOf('/')).toInt();
            QString userId = id;
            userId.remove(0, id.indexOf('/') + 1);
            NextcloudAccount activeAccount = m_accountService.getAccountById(accountId);

            QUrl endpoint = QUrl(activeAccount.host());
            endpoint.setPath(endpoint.path() + "/avatar/" + userId + "/" + QString::number(requestedSize.height()));

            QNetworkRequest request(endpoint);

            connect(&m_qnam, &QNetworkAccessManager::finished, this, &AsyncAvatarResponse::avatarReceived);

            QString concatanated = activeAccount.loginName() + ":" + activeAccount.password();
            QByteArray data = concatanated.toLocal8Bit().toBase64();
            QString authValue = "Basic " + data;

            request.setRawHeader("Authorization", authValue.toLocal8Bit());
            request.setRawHeader("OCS-APIRequest", "true");

            m_qnam.get(request);
        }

        void avatarReceived(QNetworkReply *reply) {
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
        Accounts &m_accountService = Accounts::getInstance();
};

QQuickImageResponse *AvatarProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    AsyncAvatarResponse *response = new AsyncAvatarResponse(id, requestedSize, &pool);
    return response;
}
