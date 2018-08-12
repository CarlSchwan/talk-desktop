#include "ncauthnetworkaccessmanager.h"
//#include <QtQuick>
//#include <private/qquickwebview_p.h>
//#include <private/qquickwebpage_p.h>

NcAuthNetworkAccessManager::NcAuthNetworkAccessManager(QObject *parent):
    QNetworkAccessManager( parent )
{
}

QNetworkReply *NcAuthNetworkAccessManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    qDebug() << "createRequest entered";
    QNetworkRequest req(request);
    req.setRawHeader("OCS-APIREQUEST", "true");
    req.setRawHeader("User-Agent", "Blizzz's Toy");
    qDebug() << "req was modified";
    QNetworkReply *reply = QNetworkAccessManager::createRequest( op, req, outgoingData );
    return reply;
}

void NcAuthNetworkAccessManager::replacePagesQNAM(const QVariant &v)
{
    //qDebug() << "setting NAM";
    //qDebug() << v;
    //QQuickWebView *view = qobject_cast<QQuickWebView*>(v.value<QObject*>());
    //QQuickWebPage *page = view->page();
    //view->
    //page->setNetworkAccessManager(this);
    //QObject *webView = view->rootObject();
    //qDebug() << webView->property("url");
    //qDebug() << "p1";
    //qDebug() << view;
    //qDebug() << v.page();
    //QWebPage *page;
    //qDebug() << QMetaObject::invokeMethod(view, "page", Qt::DirectConnection, Q_RETURN_ARG(QWebPage*, page));
    //qDebug() << "page " << page;
    //view->page()->setNetworkAccessManager(this);
    //QWebPage page = view->page();
    //qDebug() << "p2";
    //page->setNetworkAccessManager(this);
    //qDebug() << "NAM was set";
}
