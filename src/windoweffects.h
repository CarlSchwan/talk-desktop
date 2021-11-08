// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>

class QWindow;
class QQuickItem;

class WindowEffects : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool hasWindowSystem READ hasWindowSystem CONSTANT)

    bool hasWindowSystem() const;
public:
    explicit WindowEffects(QObject *parent = nullptr);

    Q_INVOKABLE void setBlur(QQuickItem *item, bool blur);
    Q_INVOKABLE void raiseWindow(QWindow *window);
};
