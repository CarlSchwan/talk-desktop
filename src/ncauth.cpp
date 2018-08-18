#include "ncauth.h"

NcAuth::NcAuth() : QObject()
{
}

//QNetworkRequest NcAuth::startLoginFlow(QWebView & webView, const QString url)
QNetworkRequest NcAuth::startLoginFlow(const QString url)
{
    qDebug() << "setting url to " << url;
    m_request.setUrl(url);
    qDebug() << "setting request headers";
    m_request.setRawHeader("OCS-APIREQUEST", "true");
    m_request.setRawHeader("User-Agent", "Blizzz's Toy");
    //qDebug() << "requesting at " << webView;
    //webView.load(m_request);

    m_reply = m_qnam.get(m_request);
    connect(m_reply, SIGNAL(finished()), this, SLOT(receivedLoginFlowPage()));

    return m_request;
}

void NcAuth::receivedLoginFlowPage()
{
    setContent(m_reply->readAll());
    setWebVisibility(true);
    qDebug() << "content " << m_body;
}

QString NcAuth::url() {
    return m_request.url().toString();
}
