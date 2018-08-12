#ifndef NCAUTH_H
#define NCAUTH_H

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class NcAuth: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QByteArray content READ content WRITE setContent NOTIFY contentChanged)
public:
    Q_PROPERTY(bool webVisible READ webVisible WRITE setWebVisibility NOTIFY webVisibilityChanged)
    NcAuth();
    QByteArray content() { return m_body; }
    bool webVisible() { return m_webVisible; }


signals:
    void contentChanged();
    void webVisibilityChanged();

private:
    QNetworkAccessManager m_qnam;
    QNetworkRequest m_request;
    QNetworkReply *m_reply;
    QByteArray m_body;
    bool m_webVisible = false;

    void setContent(QByteArray s) {
        m_body = s;
        emit contentChanged();
    }
    void setWebVisibility (bool b) {
        m_webVisible = b;
        emit webVisibilityChanged();
    }

public slots:
    QNetworkRequest startLoginFlow(const QString url);
    QString url();

private slots:
    void receivedLoginFlowPage();
};

#endif // NCAUTH_H
