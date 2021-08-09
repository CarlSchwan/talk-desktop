// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "roomlistfilter.h"
#include "roomservice.h"
#include <QDebug>

RoomListFilterModel::RoomListFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setFilterRole(RoomService::NameRole);
    setDynamicSortFilter(true);
    setSortRole(RoomService::LastMessageTimestampRole);
    sort(0, Qt::AscendingOrder);
}

QString RoomListFilterModel::filterText() const
{
    return m_filterText;
}

void RoomListFilterModel::setFilterText(const QString &filterText)
{
    if (filterText == m_filterText) {
        return;
    }
    m_filterText = filterText;
    Q_EMIT filterTextChanged();
    setFilterFixedString(m_filterText);
}

bool RoomListFilterModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    return sourceModel()->data(left, RoomService::LastMessageTimestampRole).toInt() >
        sourceModel()->data(right, RoomService::LastMessageTimestampRole).toInt();
}
