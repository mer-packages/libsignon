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

#ifndef SSOTEST2PLUGIN_H_
#define SSOTEST2PLUGIN_H_

#include <QtCore>

#include "ssotest2data.h"
#include "SignOn/authpluginif.h"

namespace SsoTest2PluginNS {

class SsoTest2Plugin: public AuthPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(AuthPluginInterface)
public:
    SsoTest2Plugin(QObject *parent = 0);
    virtual ~SsoTest2Plugin();

public Q_SLOTS:
    QString type() const { return m_type; }
    QStringList mechanisms() const { return m_mechanisms; }
    void cancel();
    void process(const SignOn::SessionData &inData,
                 const QString &mechanism = 0);
    void userActionFinished(const SignOn::UiSessionData &data);
    void refresh(const SignOn::UiSessionData &data);

private:
    QString m_type;
    QStringList m_mechanisms;

private Q_SLOTS:
    void execProcess(const SignOn::SessionData &inData,
                     const QString &mechanism);
};

} //namespace SsoTest2PluginNS

#endif /* SSOTEST2PLUGIN_H_ */
