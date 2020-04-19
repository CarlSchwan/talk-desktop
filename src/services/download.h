#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <QObject>
#include <QQmlEngine>
#include "accounts.h"

class Download : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Download)
    Download() {}
public:
    static QObject *qmlInstance(QQmlEngine *engine, QJSEngine *scriptEngine)
    {
        Q_UNUSED(engine);
        Q_UNUSED(scriptEngine);

        return new Download;
    }

public slots:
    void getFile(QString path, int accountId);
};

#endif // DOWNLOAD_H
