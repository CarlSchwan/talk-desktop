#include "discovery.h"
#include <QNetworkRequest>
#include <QNetworkReply>

Discovery::Discovery(QObject *parent) : QObject(parent)
{
}

void Discovery::discoverInstance(QString url)
{
    DiscoveryRun *runner = new DiscoveryRun(url, this);
    connect(runner, &DiscoveryRun::nextcloudDiscoveryFinished, this,
            &Discovery::discoveryFinished);
    runner->checkAvailability();
}

void Discovery::discoveryFinished(int result, QUrl host, QString originalUrl) {
    if(result == DiscoveryRun::DiscoveryResult::Available) {
        emit discoverySuccessful(host.toString(), originalUrl);
        return;
    }
    QString msg = "Invalid host";
    if(result == DiscoveryRun::DiscoveryResult::HostNotFound) {
        msg = "Host not found";
    }
    emit discoveryFailed(msg, host.toString(), originalUrl);
}

void Discovery::verifyCredentials(QString host, QString loginName, QString token)
{
    DiscoveryRun *runner = new DiscoveryRun(host, loginName, token, this);
    connect(runner, &DiscoveryRun::verifyCredentialsFinished, this,
            &Discovery::verifyCredentialsFinished);
    runner->verifyCredentials();
}

void Discovery::verifyCredentialsFinished(bool isVerified, QString host, QString originalUrl, QString loginName, QString token, QString userId)
{
    Q_UNUSED(originalUrl);
    if(!isVerified) {
        emit credentialsVerificationFailed();
        return;
    }
    emit credentialsVerificationSuccessful(host, loginName, token, userId);
}
