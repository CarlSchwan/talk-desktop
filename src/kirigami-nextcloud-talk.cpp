// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCommandLineParser>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QIcon>

#ifdef Q_OS_ANDROID
#include <QGuiApplication>
#else
#include <QApplication>
#endif

#include <KAboutData>
#ifdef HAVE_KDBUSADDONS
#include <KDBusService>
#endif
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KWindowConfig>

#include "talk-version.h"

#include "providers/avatarprovider.h"
#include "providers/previewprovider.h"
#include "services/accounts.h"
#include "services/download.h"
#include "services/participants.h"
#include "discovery.h"
#include "roomservice.h"
#include "nextcloudaccount.h"
#include "constants/ConversationTypeClass.h"
#include "constants/PresenceStatusClass.h"
#include "roomlistfilter.h"
#include "talkconfig.h"
#include "desktop/emojimodel.h"
#include "desktop/windoweffects.h"
#include "desktop/chatboxhelper.h"
#include "desktop/clipboard.h"
#ifdef HAVE_COLORSCHEME
#include "desktop/colorschemer.h"
#endif
#include "messageeventmodel.h"

#ifdef Q_OS_ANDROID
Q_DECL_EXPORT
#endif
int main(int argc, char *argv[])
{
    QIcon::setFallbackThemeName("breeze");
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QNetworkProxyFactory::setUseSystemConfiguration(true);

#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("Material"));
#else
    QApplication app(argc, argv);
    // Default to org.kde.desktop style unless the user forces another style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }
#endif

#ifdef Q_OS_WINDOWS
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }

    QApplication::setStyle(QStringLiteral("breeze"));
    auto font = app.font();
    font.setPointSize(10);
    app.setFont(font);
#endif

    QGuiApplication::setOrganizationName("KDE");

    KAboutData about(QStringLiteral("talk"),
                     i18n("Nextcloud Talk (unoficial)"),
                     QStringLiteral(TALK_VERSION_STRING),
                     i18n("Nextclout chat client"),
                     KAboutLicense::GPL_V3,
                     i18n("© 2018-2021 Harbour Nextcloud talk contributors"));
    about.addAuthor(i18n("Arthur Schiwon"), i18n("Maintainer"), QStringLiteral("blizzz@arthur-schiwon.de"));
    about.addAuthor(i18n("Carl Schwan"), i18n("Kiriami port developer"), QStringLiteral("carl@carlschwan.eu"));

    qmlRegisterUncreatableType<ConversationTypeClass>("harbour.nextcloud.talk", 1, 0, "ConversationType", "Enum type");
    qmlRegisterUncreatableType<PresenceStatusClass>("harbour.nextcloud.talk", 1, 0, "PresenceStatus", "Enum type");
    qmlRegisterUncreatableType<MessageEventModel>("harbour.nextcloud.talk", 1, 0, "MessageEventModel", "Enum type");

    qmlRegisterType<Discovery>("harbour.nextcloud.talk", 1, 0, "Discovery");
    qmlRegisterType<RoomListFilterModel>("harbour.nextcloud.talk", 1, 0, "RoomListFilterModel");
    qmlRegisterType<Participants>("harbour.nextcloud.talk", 1, 0, "ParticipantService");
    qmlRegisterType<MessageEventModel>("harbour.nextcloud.talk", 1, 0, "MessageEventModel");

    qmlRegisterSingletonType<Download>("harbour.nextcloud.talk", 1, 0, "DownloadService", &Download::qmlInstance);
    ChatBoxHelper chatBoxHelper;
    qmlRegisterSingletonInstance("harbour.nextcloud.talk", 1, 0, "ChatBoxHelper", &chatBoxHelper);
    WindowEffects windowEffects;
    qmlRegisterSingletonInstance("harbour.nextcloud.talk", 1, 0, "WindowEffects", &windowEffects);
    Clipboard clipboard;
    qmlRegisterSingletonInstance("harbour.nextcloud.talk", 1, 0, "Clipboard", &clipboard);

    qmlRegisterSingletonType<AccountModel>("harbour.nextcloud.talk", 1, 0, "AccountService",
        [](QQmlEngine *eng, QJSEngine *js) -> QObject *
        {
            Q_UNUSED(js)
            eng->setObjectOwnership(AccountModel::getInstance(), QQmlEngine::ObjectOwnership::CppOwnership);
            return AccountModel::getInstance();
        }
    );

    qmlRegisterSingletonType<EmojiModel>("harbour.nextcloud.talk", 1, 0, "EmojiModel", [](QQmlEngine *engine2, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(scriptEngine);
        Q_UNUSED(engine2);
        return new EmojiModel();
    });

    qmlRegisterSingletonType<RoomService>("harbour.nextcloud.talk", 1, 0, "RoomService",
        [](QQmlEngine *eng, QJSEngine *js) -> QObject *
        {
            Q_UNUSED(js)
            return new RoomService;
        }
    );

    auto config = TalkConfig::self();
    qmlRegisterSingletonInstance("harbour.nextcloud.talk", 1, 0, "Config", config);
#ifdef HAVE_COLORSCHEME
    ColorSchemer colorScheme;
    qmlRegisterSingletonInstance<ColorSchemer>("harbour.nextcloud.talk", 1, 0, "ColorSchemer", &colorScheme);
    if (!config->colorScheme().isEmpty()) {
        colorScheme.apply(config->colorScheme());
    }
#endif

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    KLocalizedString::setApplicationDomain("talk");
    QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QCoreApplication::quit);

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("Client for the talk communication protocol"));

    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    engine.addImageProvider("avatar", new AvatarProvider);
    engine.addImageProvider("preview", new PreviewProvider);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
