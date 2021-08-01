#ifndef SECRETS_H
#define SECRETS_H

#include <QByteArray>
#include <memory>
#ifndef KDE_EDITION
#include <Sailfish/Secrets/request.h>
#include <Sailfish/Secrets/secretmanager.h>
#include <Sailfish/Secrets/secret.h>
#endif

class Secrets
{
public:
    Secrets();
    QByteArray get(const QString &key);
    bool set(const QString &key, const QByteArray &data);
    bool unset(const QString &key);

private:
#ifndef KDE_EDITION
    std::unique_ptr<Sailfish::Secrets::SecretManager> m_manager{new Sailfish::Secrets::SecretManager()};
    Sailfish::Secrets::Secret::Identifier createIdentifier(const QString &key);
    bool checkResult(const Sailfish::Secrets::Request &req);
    void ensureCollection();
    static const QString m_collectionName;

    bool setupCollection();
#endif
};

#endif // SECRETS_H
