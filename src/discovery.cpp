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
        emit discoverySuccessful(host.scheme() + "://" + host.host(), originalUrl);
        return;
    }
    QString msg = "Invalid host";
    if(result == DiscoveryRun::DiscoveryResult::HostNotFound) {
        msg = "Host not found";
    }
    emit discoveryFailed(msg, host.scheme() + "://" + host.host(), originalUrl);
}
