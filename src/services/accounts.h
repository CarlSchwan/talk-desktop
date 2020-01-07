#ifndef ACCOUNTS_H
#define ACCOUNTS_H

#include <QAbstractListModel>
#include <QStringList>
#include <QVector>
#include "../nextcloudaccount.h"

class Accounts : public QAbstractListModel
{
    Q_OBJECT
public:
    enum AccountRoles {
        NameRole = Qt::UserRole + 1,
        AccountRole = Qt::UserRole + 2,
    };
    explicit Accounts(QObject *parent = nullptr);
    static Accounts& getInstance();
    NextcloudAccount getAccountById(const int id);
    QVector<NextcloudAccount> getAccounts();

    // Basic functionality:
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;

public slots:
    void addAccount(QString url, QString loginName, QString token, QString userId);
    void loadAccounts();

private:
    //Accounts(QObject *parent = nullptr);
    Accounts(const Accounts&);
    const Accounts& operator=(const Accounts&);
    QVector<NextcloudAccount> readAccounts();
    QVector<NextcloudAccount> m_accounts;
    bool is_initialized = false;
    int max_id = 0;

};

#endif // ACCOUNTS_H
