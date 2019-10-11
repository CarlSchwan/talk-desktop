#ifndef CHAT_H
#define CHAT_H

#include <QAbstractListModel>

class Chat : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit Chat(QObject *parent = nullptr);
};

#endif // CHAT_H
