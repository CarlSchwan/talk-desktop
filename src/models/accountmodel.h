// SPDX-FileCopyrightText: 2018 Arthur Schiwon <blizzz@arthur-schiwon.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../nextcloudaccount.h"
#include "../services/secrets.h"
#include <QAbstractListModel>
#include <QStringList>
#include <QVector>

class AccountModel : public QAbstractListModel
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
    static AccountModel *getInstance();
    QVector<NextcloudAccount *> getAccounts() const;
    int getAccountId(NextcloudAccount *account) const;
    NextcloudAccount *getAccountById(int accountId) const;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

public Q_SLOTS:
    void load();
    void addAccount(const QString &url, const QString &loginName, const QString &token, const QString &userId);
    void deleteAccount(int accountId);

private:
    explicit AccountModel(QObject *parent = nullptr);
    QVector<NextcloudAccount *> m_accounts;
    Secrets m_secrets;
    int m_maxId = 0;
};
