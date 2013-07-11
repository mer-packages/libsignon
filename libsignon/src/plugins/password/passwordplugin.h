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
#ifndef PASSWORDPLUGIN_H_
#define PASSWORDPLUGIN_H_

#include <QtCore>

#include "SignOn/sessiondata.h"
#include "SignOn/uisessiondata.h"
#include "SignOn/authpluginif.h"

namespace PasswordPluginNS {

/*!
 * @class PasswordPlugin
 * @brief Password plugin for Sign-On.
 * Sign-On plugin that returns plain text password.
 * If password is not given in parameters, user is requested to input password.
 */
class PasswordPlugin: public AuthPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(AuthPluginInterface)

public:
    PasswordPlugin(QObject *parent = 0);
    virtual ~PasswordPlugin();

public Q_SLOTS:
    QString type() const;
    QStringList mechanisms() const;
    void cancel();
    void process(const SignOn::SessionData &inData,
                 const QString &mechanism = 0);
    void userActionFinished(const SignOn::UiSessionData &data);
//    void refresh(const SignOn::UiSessionData &data);
    };

} //namespace PasswordPluginNS

#endif /* PASSWORDPLUGIN_H_ */
