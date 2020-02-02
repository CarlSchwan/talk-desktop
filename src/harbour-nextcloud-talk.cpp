#include <QtQuick>

#include <sailfishapp.h>
#include "services/accounts.h"
#include "discovery.h"
#include "roomservice.h"

int main(int argc, char *argv[])
{
    // SailfishApp::main() will display "qml/harbour-nextcloud-talk.qml", if you need more
    // control over initialization, you can use:
    //
    //   - SailfishApp::application(int, char *[]) to get the QGuiApplication *
    //   - SailfishApp::createView() to get a new QQuickView * instance
    //   - SailfishApp::pathTo(QString) to get a QUrl to a resource file
    //   - SailfishApp::pathToMainQml() to get a QUrl to the main QML file
    //
    // To display the view, call "show()" (will show fullscreen on device).

    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));

    QScopedPointer<QQuickView> v(SailfishApp::createView());

    qmlRegisterType<Discovery>("harbour.nextcloud.talk", 1, 0, "Discovery");
    qmlRegisterType<RoomService>("harbour.nextcloud.talk", 1, 0, "RoomService");

    qmlRegisterSingletonType<Accounts>("harbour.nextcloud.talk", 1, 0, "AccountService",
        [](QQmlEngine *eng, QJSEngine *js) -> QObject *
        {
            eng->setObjectOwnership(&Accounts::getInstance(), QQmlEngine::ObjectOwnership::CppOwnership);
            return &Accounts::getInstance();
        }
    );

    v->setSource(SailfishApp::pathTo("qml/harbour-nextcloud-talk.qml"));
    v->show();
    return app->exec();
}
