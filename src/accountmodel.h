#ifndef ACCOUNTMODEL_H
#define ACCOUNTMODEL_H

#include <QAbstractListModel>
#include "nextcloudaccount.h"

class AccountModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum AccountRoles {
        NameRole = Qt::UserRole + 1,
        AccountRole = Qt::UserRole + 2,
    };

    explicit AccountModel(QObject *parent = nullptr);

    // Basic functionality:
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;

public slots:
    void addAccount(QString url, QString loginName, QString token, QString userId);
    void loadAccounts();

private:
    bool is_initialized = false;
    int max_id = 0;
    QVector<NextcloudAccount> accounts;

};

#endif // ACCOUNTMODEL_H
