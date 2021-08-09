#ifndef ACCOUNTS_H
#define ACCOUNTS_H

#include <QAbstractListModel>
#include <QStringList>
#include <QVector>
#include "../nextcloudaccount.h"
#include "secrets.h"

class Accounts : public QAbstractListModel
{
    Q_OBJECT
public:
    enum AccountRoles {
        NameRole = Qt::UserRole + 1,
        AccountRole,
        LogoRole,
        InstanceNameRole,
        ColorRole,
        ColorModeRole,
    };
    enum ColorMode {
        InstanceColor,
        OverriddenColor,
    };
    Q_ENUM(AccountRoles)
    Q_ENUM(ColorMode)
    explicit Accounts(QObject *parent = nullptr);
    static Accounts* getInstance();
    NextcloudAccount* getAccountById(const int id);
    QVector<NextcloudAccount*> getAccounts();

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void addAccount(QString url, QString loginName, QString token, QString userId);
    void deleteAccount(int accountId);
    void loadAccounts();

private:
    Accounts(const Accounts*);
    QVector<NextcloudAccount*> readAccounts();
    QVector<NextcloudAccount*> m_accounts;
    bool is_initialized = false;
    int max_id = 0;
    Secrets m_secrets;

};

#endif // ACCOUNTS_H
