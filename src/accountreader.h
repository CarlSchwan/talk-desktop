#ifndef ACCOUNTREADER_H
#define ACCOUNTREADER_H

#include <QVector>
#include "nextcloudaccount.h"

class AccountReader
{
public:
    AccountReader();

public slots:
    static QVector<NextcloudAccount> readAccounts();
};

#endif // ACCOUNTREADER_H
