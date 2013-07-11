/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
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
#ifndef TESTAUTHSERVICERESULT_H
#define TESTAUTHSERVICERESULT_H

#include <QObject>

#include "SignOn/authservice.h"
#include "SignOn/identity.h"
#include "SignOn/signonerror.h"

using namespace SignOn;

class TestAuthServiceResult: public QObject
{
    Q_OBJECT

public :
    enum ResponseType
    {
        NormalResp = 0,
        ErrorResp,
        InexistentResp
    };

public:
    Error::ErrorType m_error;
    QString m_errMsg;

    ResponseType m_responseReceived;

    QList<IdentityInfo> m_identities;
    QStringList m_methods;
    QPair<QString, QStringList> m_mechanisms;
    bool m_cleared;
    QString m_queriedMechsMethod;

public:
    TestAuthServiceResult();
    void reset();

public Q_SLOTS:
    void error(const SignOn::Error &err);
    void methodsAvailable(const QStringList &methods);
    void mechanismsAvailable(const QString &method,
                             const QStringList &mechanisms);
    void identities(const QList<SignOn::IdentityInfo> &identityList);
    void cleared();

Q_SIGNALS:
    void testCompleted();
};


#endif // TESTAUTHSERVICERESULT_H
