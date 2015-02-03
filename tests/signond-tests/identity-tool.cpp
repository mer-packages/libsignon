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
#include <QTextStream>
#include <SignOn/Identity>

using namespace SignOn;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);

    IdentityInfo info;

    QStringList args = QCoreApplication::arguments();
    for (int i = 1; i < args.count(); i++) {
        if (args[i] == "--acl") {
            info.setAccessControlList(args[++i].split(','));
        } else if (args[i] == "--method") {
            QString method = args[++i];
            QStringList mechanisms = args[++i].split(',');
            info.setMethod(method, mechanisms);
        } else if (args[i] == "--caption") {
            info.setCaption(args[++i]);
        }
    }

    Identity *identity = Identity::newIdentity(info);
    Q_ASSERT(identity != NULL);

    QObject::connect(identity, SIGNAL(error(const SignOn::Error&)),
                     &app, SLOT(quit()));
    QObject::connect(identity, SIGNAL(credentialsStored(const quint32)),
                     &app, SLOT(quit()));
    identity->storeCredentials();
    app.exec();
    Q_ASSERT(identity->id() != 0);

    out << identity->id();
    return 0;
}
