#include "abstractnextcloudimageprovider.h"

#include <QException>
#include <QNetworkReply>
#include "../services/accounts.h"

class AsyncImageResponse : public QQuickImageResponse
{
    public:
        AsyncImageResponse(QNetworkRequest request, QThreadPool *pool)
            : m_accountService(AccountModel::getInstance())
        {
            auto reply = m_qnam.get(request);
            connect(reply, &QNetworkReply::finished, this, [this, reply]() {
                m_image.loadFromData(reply->readAll());
                emit finished();
            });
        }

        QQuickTextureFactory *textureFactory() const override
        {
            return QQuickTextureFactory::textureFactoryForImage(m_image);
        }

    private:
        QImage m_image;
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
    AsyncImageResponse *response = new AsyncImageResponse(request, &pool);
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
