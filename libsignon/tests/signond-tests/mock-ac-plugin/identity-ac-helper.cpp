/*
 * This file is part of signon
 *
 * Copyright (C) 2013 Canonical Ltd.
 *
 * Contact: Alberto Mardegan <alberto.mardegan@canonical.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <QCoreApplication>
#include <QDebug>
#include <SignOn/Identity>

using namespace SignOn;

class Helper: public QObject
{
    Q_OBJECT

public:
    Helper(quint32 id, const QString &appId);

private Q_SLOTS:
    void onError(const SignOn::Error &error);
    void onInfo(const SignOn::IdentityInfo &info);
    void onCredentialsStored();

private:
    Identity *m_identity;
    QString m_appId;
};

Helper::Helper(quint32 id, const QString &appId):
    QObject(),
    m_identity(0),
    m_appId(appId)
{
    m_identity = Identity::existingIdentity(id);
    Q_ASSERT(m_identity != NULL);

    QObject::connect(m_identity, SIGNAL(info(const SignOn::IdentityInfo&)),
                     this, SLOT(onInfo(const SignOn::IdentityInfo&)));
    QObject::connect(m_identity, SIGNAL(error(const SignOn::Error&)),
                     this, SLOT(onError(const SignOn::Error&)));

    m_identity->queryInfo();
}

void Helper::onInfo(const SignOn::IdentityInfo &info)
{
    /* If the identity caption has the magic value of "allow", then update the
     * ACL and return true. */
    bool isAllowed = (info.caption() == "allow");

    if (!isAllowed) {
        QCoreApplication::exit(EXIT_FAILURE);
        return;
    }

    QObject::connect(m_identity, SIGNAL(credentialsStored(const quint32)),
                     this, SLOT(onCredentialsStored()));
    IdentityInfo updated = info;
    updated.setAccessControlList(info.accessControlList() << m_appId);
    m_identity->storeCredentials(updated);
}

void Helper::onCredentialsStored()
{
    qDebug() << Q_FUNC_INFO;
    QCoreApplication::exit(EXIT_SUCCESS);
}

void Helper::onError(const SignOn::Error &error)
{
    qDebug() << Q_FUNC_INFO << error.type() << error.message();
    QCoreApplication::exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QStringList args = QCoreApplication::arguments();
    uint id = args.at(1).toUInt();
    QString appId = args.at(2);
    qDebug() << "Started" << id << appId;

    Helper helper(quint32(id), appId);
    return app.exec();
}

#include "identity-ac-helper.moc"
