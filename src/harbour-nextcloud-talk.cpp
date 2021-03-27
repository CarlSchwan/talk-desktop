#include <QtQuick>

#include <sailfishapp.h>
#include "providers/avatarprovider.h"
#include "providers/previewprovider.h"
#include "services/accounts.h"
#include "services/download.h"
#include "services/participants.h"
#include "services/notifications.h"
#include "discovery.h"
#include "roomservice.h"
#include "nextcloudaccount.h"
#include "constants/ConversationTypeClass.h"
#include "constants/PresenceStatusClass.h"

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

    qmlRegisterUncreatableType<ConversationTypeClass>("harbour.nextcloud.talk", 1, 0, "ConversationType", "Enum type");
    qmlRegisterUncreatableType<PresenceStatusClass>("harbour.nextcloud.talk", 1, 0, "PresenceStatus", "Enum type");

    qmlRegisterType<Discovery>("harbour.nextcloud.talk", 1, 0, "Discovery");
    qmlRegisterType<RoomService>("harbour.nextcloud.talk", 1, 0, "RoomService");
    qmlRegisterType<Participants>("harbour.nextcloud.talk", 1, 0, "ParticipantService");

    qmlRegisterSingletonType<Download>("harbour.nextcloud.talk", 1, 0, "DownloadService", &Download::qmlInstance);

    qmlRegisterSingletonType<Accounts>("harbour.nextcloud.talk", 1, 0, "AccountService",
        [](QQmlEngine *eng, QJSEngine *js) -> QObject *
        {
            Q_UNUSED(js)
            eng->setObjectOwnership(Accounts::getInstance(), QQmlEngine::ObjectOwnership::CppOwnership);
            return Accounts::getInstance();
        }
    );

    Notifications notificationService;
    notificationService.watchAccounts(Accounts::getInstance());

    v->engine()->addImageProvider("avatar", new AvatarProvider);
    v->engine()->addImageProvider("preview", new PreviewProvider);

    v->setSource(SailfishApp::pathTo("qml/harbour-nextcloud-talk.qml"));
    v->show();
    return app->exec();
}
