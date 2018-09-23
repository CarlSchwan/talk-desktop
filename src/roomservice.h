#ifndef ROOMSERVICE_H
#define ROOMSERVICE_H

#include <QAbstractListModel>
#include <QNetworkAccessManager>
#include "accountreader.h"
#include "nextcloudaccount.h"
#include "room.h"

class RoomService : public QAbstractListModel, AccountReader
{
    Q_OBJECT
public:
    enum RoomRoles {
        NameRole = Qt::UserRole + 1,
    };

    explicit RoomService(QObject *parent = nullptr);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;

public slots:
    void loadRooms();
    void roomsLoadedFromAccount(QNetworkReply *reply);

private:
    QVector<NextcloudAccount> m_accounts;
    QVector<Room> m_rooms;
    QNetworkAccessManager m_nam;
    int m_pendingRequests = 0;
};

#endif // ROOMSERVICE_H
