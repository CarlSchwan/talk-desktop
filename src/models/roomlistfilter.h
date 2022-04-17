// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QSortFilterProxyModel>

class RoomListFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
public:
    RoomListFilterModel(QObject *parent = nullptr);

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    QString filterText() const;
    void setFilterText(const QString &filterText);

Q_SIGNALS:
    void filterTextChanged();

private:
    QString m_filterText;
};
