#ifndef SECRETS_H
#define SECRETS_H

#include <QByteArray>
#include <memory>
#include <Sailfish/Secrets/request.h>
#include <Sailfish/Secrets/secretmanager.h>
#include <Sailfish/Secrets/secret.h>

class Secrets
{
public:
    Secrets();
    QByteArray get(const QString &key);
    bool set(const QString &key, const QByteArray &data);
    bool unset(const QString &key);

private:
    std::unique_ptr<Sailfish::Secrets::SecretManager> m_manager{new Sailfish::Secrets::SecretManager()};
    static const QString m_collectionName;

    void ensureCollection();
    bool setupCollection();
    bool checkResult(const Sailfish::Secrets::Request &req);
    Sailfish::Secrets::Secret::Identifier createIdentifier(const QString &key);

};

#endif // SECRETS_H
