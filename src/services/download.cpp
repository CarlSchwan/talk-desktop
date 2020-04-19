#include "download.h"
#include "QDesktopServices"
#include "QDebug"

void Download::getFile(QString path, int accountId)
{
    Accounts &accountService = Accounts::getInstance();
    NextcloudAccount account = accountService.getAccountById(accountId);

    QUrl endpoint = QUrl(account.host());
    endpoint.setPath(endpoint.path() + "/remote.php/webdav/" + path);
    endpoint.setUserName(account.loginName());
    endpoint.setPassword(account.password());
    QDesktopServices::openUrl(endpoint);
}
