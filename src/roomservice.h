#ifndef ROOMSERVICE_H
#define ROOMSERVICE_H

#include <QAbstractListModel>
#include <QException>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QTimer>
#include "accountreader.h"
#include "nextcloudaccount.h"
#include "room.h"

class RoomService : public QAbstractListModel, AccountReader
{
    Q_OBJECT
public:
    enum RoomRoles {
        NameRole = Qt::UserRole + 1,
        TokenRole = Qt::UserRole + 2,
        AccountRole = Qt::UserRole + 3,
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
    void stopPolling();
    void sendMessage(QString messageText);

signals:
    void newMessage(QString message);

private slots:
    void pollRoom();
    NextcloudAccount getAccountById(const int id);
    void roomPolled(QNetworkReply *reply);

private:
    QVector<NextcloudAccount> m_accounts;
    QVector<Room> m_rooms;
    QNetworkAccessManager m_nam;
    QNetworkAccessManager namPosting;
    int m_pendingRequests = 0;
    QTimer m_pollTimer;
    QString activeToken;
    int activeAccountId;
    bool isPolling = false;
    int lastKnownMessageId = 0;
};

#endif // ROOMSERVICE_H
