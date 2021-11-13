#ifndef PREVIEWPROVIDER_H
#define PREVIEWPROVIDER_H

#include "abstractnextcloudimageprovider.h"

class PreviewProvider : public AbstractNextcloudImageProvider
{
public:
    PreviewProvider();
    QNetworkRequest getRequest(const QString &subject, NextcloudAccount *account, const QSize &requestedSize);
};

#endif // PREVIEWPROVIDER_H
