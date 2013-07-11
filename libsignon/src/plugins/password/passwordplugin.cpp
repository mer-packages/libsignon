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

#include "passwordplugin.h"
#include "SignOn/signonplugincommon.h"

using namespace SignOn;

namespace PasswordPluginNS {

PasswordPlugin::PasswordPlugin(QObject *parent):
    AuthPluginInterface(parent)
{
    TRACE();
}

PasswordPlugin::~PasswordPlugin()
{
    TRACE();
}

QString PasswordPlugin::type() const
{
    return QLatin1String("password");
}

QStringList PasswordPlugin::mechanisms() const
{
    QStringList res = QStringList(QLatin1String("password"));

    return res;
}

void PasswordPlugin::cancel()
{
    emit error(Error(Error::SessionCanceled));
}

/*
 * Password plugin is used for returning password
 * */
void PasswordPlugin::process(const SignOn::SessionData &inData,
                            const QString &mechanism )
{
    TRACE();
    Q_UNUSED(mechanism);
    SignOn::SessionData response;

    if (!inData.UserName().isEmpty())
        response.setUserName(inData.UserName());

    if (!inData.Secret().isEmpty()) {
        response.setSecret(inData.Secret());
        emit result(response);
        return;
    }

    //we didn't receive password from signond, so ask from user
    SignOn::UiSessionData data;
    if (inData.UserName().isEmpty())
        data.setQueryUserName(true);
    else
        data.setUserName(inData.UserName());

    data.setQueryPassword(true);
    emit userActionRequired(data);

    return;
}

void PasswordPlugin::userActionFinished(const SignOn::UiSessionData &data)
{
    TRACE();

    if (data.QueryErrorCode() == QUERY_ERROR_NONE) {
        SignOn::SessionData response;
        response.setUserName(data.UserName());
        response.setSecret(data.Secret());
        emit result(response);
        return;
    }

    if (data.QueryErrorCode() == QUERY_ERROR_CANCELED)
        emit error(Error::SessionCanceled);
    else
        emit error(Error(Error::UserInteraction,
                   QLatin1String("userActionFinished error: ")
                   + QString::number(data.QueryErrorCode())));

    return;
}

SIGNON_DECL_AUTH_PLUGIN(PasswordPlugin)

} //namespace PasswordPluginNS
