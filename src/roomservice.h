#ifndef ROOMSERVICE_H
#define ROOMSERVICE_H

#include <QAbstractListModel>
#include <QException>
#include <QNetworkAccessManager>
#include "db.h"
#include "nextcloudaccount.h"
#include "room.h"
#include "services/accounts.h"

class RoomService : public QAbstractListModel
{
    Q_OBJECT
public:
    enum RoomRoles {
        NameRole = Qt::UserRole + 1,
        TokenRole = Qt::UserRole + 2,
        AccountRole = Qt::UserRole + 3,
        UnreadRole = Qt::UserRole + 4,
        MentionedRole = Qt::UserRole + 5,
        UserIdRole = Qt::UserRole + 6,
    };

    explicit RoomService(QObject *parent = nullptr);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;

public slots:
    void loadRooms();
    void roomsLoadedFromAccount(QNetworkReply *reply);
    Room getRoom(QString token, int accountId);
    void startPolling(QString token, int accountId);
    bool isPolling(QString token, int accountId);
    void stopPolling();
    void sendMessage(QString messageText, int replyToId);

signals:
    void newMessage(QString message);

private slots:
    void pollRoom();
    void roomPolled(QNetworkReply *reply);
    Room findRoomByTokenAndAccount(const QString token, const int accountId);
    void onAccountsChanged();
    void emitAfterActiveRoomChanged(QString token, int accountId);

private:
    Accounts* m_accountService = Accounts::getInstance();
    QVector<Room> m_rooms;
    QVector<QNetworkReply*> m_rooms_requests;
    QNetworkAccessManager m_nam;
    QNetworkAccessManager namPosting;
    int m_pendingRequests = 0;
    QString activeToken;
    int activeAccountId;
    bool m_isPolling = false;
    Db m_db;
    int m_lookIntoFuture = 0;
};

#endif // ROOMSERVICE_H
