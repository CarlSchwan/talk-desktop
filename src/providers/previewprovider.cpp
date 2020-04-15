#include "previewprovider.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include "../services/accounts.h"

PreviewProvider::PreviewProvider() : QQuickAsyncImageProvider()
{

}

class AsyncPreviewResponse : public QQuickImageResponse
{
    public:
        AsyncPreviewResponse(const QString &id, const QSize &requestedSize, QThreadPool *pool)
        {
            const int accountId = id.left(id.indexOf('/')).toInt();
            QString fileId = id;
            fileId.remove(0, id.indexOf('/') + 1);
            NextcloudAccount activeAccount = m_accountService.getAccountById(accountId);

            QUrl endpoint = QUrl(activeAccount.host());
            endpoint.setPath(endpoint.path() + "/core/preview");
            endpoint.setQuery("fileId=" + fileId
                              + "&x=" + QString::number(requestedSize.width())
                              + "&y=" + QString::number(requestedSize.height())
                              );

            QNetworkRequest request(endpoint);

            connect(&m_qnam, &QNetworkAccessManager::finished, this, &AsyncPreviewResponse::previewReceived);

            QString concatanated = activeAccount.loginName() + ":" + activeAccount.password();
            QByteArray data = concatanated.toLocal8Bit().toBase64();
            QString authValue = "Basic " + data;

            request.setRawHeader("Authorization", authValue.toLocal8Bit());
            request.setRawHeader("OCS-APIRequest", "true");

            m_qnam.get(request);
        }

        void previewReceived(QNetworkReply *reply) {
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

QQuickImageResponse *PreviewProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    AsyncPreviewResponse *response = new AsyncPreviewResponse(id, requestedSize, &pool);
    return response;
}
