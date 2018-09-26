#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <sailfishapp.h>
#include "ncauth.h"
#include "ncauthnamf.h"
#include "accountmodel.h"
#include "discovery.h"
#include "roomservice.h"
#include "chat.h"

int main(int argc, char *argv[])
{
    // SailfishApp::main() will display "qml/sfos-nextcloud-login-flow-demo.qml", if you need more
    // control over initialization, you can use:
    //
    //   - SailfishApp::application(int, char *[]) to get the QGuiApplication *
    //   - SailfishApp::createView() to get a new QQuickView * instance
    //   - SailfishApp::pathTo(QString) to get a QUrl to a resource file
    //   - SailfishApp::pathToMainQml() to get a QUrl to the main QML file
    //
    // To display the view, call "show()" (will show fullscreen on device).

    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));

    //QQmlApplicationEngine e;

    //e.setNetworkAccessManagerFactory(new QmlNetworkAccessManagerFactory);

    QScopedPointer<QQuickView> v(SailfishApp::createView());

    qmlRegisterType<NcAuth>("sfos.nextcloud.login.flow.demo", 1, 0, "NcAuth");
    qmlRegisterType<AccountModel>("sfos.nextcloud.login.flow.demo", 1, 0, "AccountModel");
    qmlRegisterType<Discovery>("sfos.nextcloud.login.flow.demo", 1, 0, "Discovery");
    qmlRegisterType<RoomService>("sfos.nextcloud.login.flow.demo", 1, 0, "RoomService");
    qmlRegisterType<Chat>("sfos.nextcloud.login.flow.demo", 1, 0, "Chat");
    //qmlRegisterType<NcAuthNetworkAccessManager>("sfos.nextcloud.login.flow.demo", 1, 0, "NcAuthNAM");

    //QQmlEngine *e = v->engine();
    //QmlNetworkAccessManagerFactory *qnamFactory = new QmlNetworkAccessManagerFactory;
    //e->setNetworkAccessManagerFactory(new QmlNetworkAccessManagerFactory);

    NcAuthNAMF namf;
    v->engine()->setNetworkAccessManagerFactory(&namf);

    //e.setNetworkAccessManagerFactory(&namf);
    //v->rootContext()->setContextProperty("networkManager", v->engine()->networkAccessManager());

    v->setSource(SailfishApp::pathTo("qml/sfos-nextcloud-login-flow-demo.qml"));
    v->show();
    return app->exec();
}
