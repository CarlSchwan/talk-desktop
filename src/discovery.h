#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>
#include "discoveryrun.h"

class Discovery : public QObject
{
    Q_OBJECT

public:
    explicit Discovery(QObject *parent = nullptr);

public slots:
    void discoverInstance(QString url);
    void discoveryFinished(int result, QUrl host, QString originalUrl);

signals:
    void protocolUnsupported(QString originalHost);
    void discoverySuccessful(QString host, QString originalHost);
    void discoveryFailed(QString message, QString host, QString originalHost);

private:
    QNetworkAccessManager m_nam;
};

#endif // DISCOVERY_H
