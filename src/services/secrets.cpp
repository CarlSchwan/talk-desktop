#include "secrets.h"
#include <QDebug>
#include <QException>
#ifndef KDE_EDITION
#include <Sailfish/Secrets/collectionnamesrequest.h>
#include <Sailfish/Secrets/createcollectionrequest.h>
#include <Sailfish/Secrets/deletesecretrequest.h>
#include <Sailfish/Secrets/storesecretrequest.h>
#include <Sailfish/Secrets/storedsecretrequest.h>
#include <Sailfish/Secrets/deletecollectionrequest.h>
using namespace Sailfish::Secrets;

const QString Secrets::m_collectionName(QStringLiteral("nextcloudTalk"));
#else
#include <qt5keychain/keychain.h>
#include <QEventLoop>
#include <QGuiApplication>
#endif

Secrets::Secrets() = default;

QByteArray Secrets::get(const QString &key) {
#ifdef KDE_EDITION
    QKeychain::ReadPasswordJob job(qAppName());
    job.setAutoDelete(false);
    job.setKey(key);
    QEventLoop loop;
    QKeychain::ReadPasswordJob::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    if (job.error() == QKeychain::Error::NoError) {
        return job.binaryData();
    }
#else
    ensureCollection();
    StoredSecretRequest ssr;
    ssr.setManager(m_manager.get());
    ssr.setUserInteractionMode(Sailfish::Secrets::SecretManager::SystemInteraction);
    ssr.setIdentifier(createIdentifier(key));;
    ssr.startRequest();
    ssr.waitForFinished();

    auto success = checkResult(ssr);
    if (success)
    {
        return ssr.secret().data();
    }

#endif
    return QByteArray();
}

bool Secrets::set(const QString &key, const QByteArray &data) {
#ifdef KDE_EDITION
    QKeychain::WritePasswordJob job(qAppName());
    job.setAutoDelete(false);
    job.setKey(key);
    job.setBinaryData(data);
    QEventLoop loop;
    QKeychain::WritePasswordJob::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();
    return job.error();
#else
    ensureCollection();

    Secret secret(createIdentifier(key));
    secret.setData(data);

    StoreSecretRequest ssr;
    ssr.setManager(m_manager.get());
    ssr.setSecretStorageType(StoreSecretRequest::CollectionSecret);
    ssr.setUserInteractionMode(SecretManager::SystemInteraction);
    ssr.setSecret(secret);
    ssr.startRequest();
    ssr.waitForFinished();

    return checkResult(ssr);
#endif
}

bool Secrets::unset(const QString &key) {
#ifdef KDE_EDITION
    QKeychain::DeletePasswordJob job(qAppName());
    job.setAutoDelete(true);
    job.setKey(key);
    QEventLoop loop;
    QKeychain::DeletePasswordJob::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();
    return job.error();
#else

    ensureCollection();

    DeleteSecretRequest dsr;
    dsr.setManager(m_manager.get());
    dsr.setUserInteractionMode(SecretManager::SystemInteraction);
    dsr.setIdentifier(createIdentifier(key));
    dsr.startRequest();
    dsr.waitForFinished();

    return checkResult(dsr);
#endif
}

#ifndef KDE_EDITION
void Secrets::ensureCollection() {
    CollectionNamesRequest cnr;
    cnr.setManager(m_manager.get());
    cnr.setStoragePluginName(SecretManager::DefaultEncryptedStoragePluginName);
    cnr.startRequest();
    cnr.waitForFinished();
    if(checkResult(cnr) && cnr.collectionNames().contains(m_collectionName)) {
        return;
    }

    bool initialized = setupCollection();
    if(!initialized) {
        QException e;
        throw e;
    }
}

bool Secrets::setupCollection()
{
    CreateCollectionRequest ccr;
    ccr.setManager(m_manager.get());
    ccr.setCollectionName(m_collectionName);
    ccr.setAccessControlMode(SecretManager::OwnerOnlyMode);
    ccr.setCollectionLockType(CreateCollectionRequest::DeviceLock);
    ccr.setDeviceLockUnlockSemantic(SecretManager::DeviceLockVerifyLock);
    ccr.setStoragePluginName(SecretManager::DefaultEncryptedStoragePluginName);
    ccr.setEncryptionPluginName(SecretManager::DefaultEncryptedStoragePluginName);
    ccr.startRequest();
    ccr.waitForFinished();
    return checkResult(ccr);
}

bool Secrets::checkResult(const Request &req)
{
    Result result = req.result();
    bool success = result.errorCode() == Result::NoError;
    if (!success) {
        qDebug() << result.errorMessage();
    }
    return success;
}

Secret::Identifier Secrets::createIdentifier(const QString &key)
{
    return Secret::Identifier(key, Secrets::m_collectionName, SecretManager::DefaultEncryptedStoragePluginName);
}
#endif
