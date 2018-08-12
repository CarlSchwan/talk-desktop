#ifndef NCAUTHNETWORKACCESSMANAGER_H
#define NCAUTHNETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>
//#include <QWebPage>
#include <QNetworkRequest>
#include <QNetworkReply>
//#include <QWebView>

class NcAuthNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT

public:
    explicit NcAuthNetworkAccessManager(QObject *parent = 0);

protected:
    virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData);

public slots:
    void replacePagesQNAM(const QVariant &view);
};

#endif // NCAUTHNETWORKACCESSMANAGER_H
