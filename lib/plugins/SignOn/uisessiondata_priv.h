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
#ifndef UISESSIONDATA_PRIV_H
#define UISESSIONDATA_PRIV_H

#define SSOUI_KEY_ERROR           QLatin1String("QueryErrorCode")
#define SSOUI_KEY_CAPTION         QLatin1String("Caption")
#define SSOUI_KEY_MESSAGEID       QLatin1String("QueryMessageId")
#define SSOUI_KEY_MESSAGE         QLatin1String("QueryMessage")
#define SSOUI_KEY_QUERYUSERNAME   QLatin1String("QueryUserName")
#define SSOUI_KEY_USERNAME        QLatin1String("UserName")
#define SSOUI_KEY_QUERYPASSWORD   QLatin1String("QueryPassword")
#define SSOUI_KEY_PASSWORD        QLatin1String("Secret")
#define SSOUI_KEY_REMEMBER        QLatin1String("RememberPassword")
#define SSOUI_KEY_SHOWREALM       QLatin1String("ShowRealm")
#define SSOUI_KEY_REALM           QLatin1String("Realm")
#define SSOUI_KEY_NETWORKPROXY    QLatin1String("NetworkProxy")
#define SSOUI_KEY_UIPOLICY        QLatin1String("UiPolicy")
#define SSOUI_KEY_OPENURL         QLatin1String("OpenUrl")
#define SSOUI_KEY_FINALURL        QLatin1String("FinalUrl")
#define SSOUI_KEY_URLRESPONSE     QLatin1String("UrlResponse")
#define SSOUI_KEY_CAPTCHAURL      QLatin1String("CaptchaUrl")
#define SSOUI_KEY_CAPTCHAIMG      QLatin1String("CaptchaImage") //QByteArray !!!
#define SSOUI_KEY_CAPTCHARESP     QLatin1String("CaptchaResponse")
#define SSOUI_KEY_REQUESTID       QLatin1String("requestId") //id of request, used for cancellation
#define SSOUI_KEY_REFRESH         QLatin1String("refreshRequired") //id of request, used for cancellation
#define SSOUI_KEY_WATCHDOG        QLatin1String("watchdog")         // automatic behavior of dialog
#define SSOUI_KEY_STORED_IDENTITY QLatin1String("StoredIdentity") /* flag whether
                                                                     the credentials are stored or not */
#define SSOUI_KEY_IDENTITY        QLatin1String("Identity") // Credentials ID
#define SSOUI_KEY_FORGOTPASSWORD    QLatin1String("ForgotPassword")
#define SSOUI_KEY_FORGOTPASSWORDURL QLatin1String("ForgotPasswordUrl")
#define SSOUI_KEY_REPLYCOOKIES      QLatin1String("ReplyCookies")
#define SSOUI_KEY_WINDOWID          QLatin1String("WindowId")
#define SSOUI_KEY_CONFIRM           QLatin1String("Confirm")
#define SSOUI_KEY_ICON              QLatin1String("Icon")
#define SSOUI_KEY_TITLE             QLatin1String("Title")
#define SSOUI_KEY_CONFIRMCOUNT      QLatin1String("ConfirmCount")
/* Embed the signon-ui in the window given by SSOUI_KEY_WINDOWID */
#define SSOUI_KEY_EMBEDDED          QLatin1String("Embedded")

/* Contains the original request parameters, as sent from the application */
#define SSOUI_KEY_CLIENT_DATA       QLatin1String("ClientData")

/* Method and mechanism; these could be used, along with the IDENTITY
 * and CLIENT_DATA keys above, to replay the authentication. */
#define SSOUI_KEY_METHOD            QLatin1String("Method")
#define SSOUI_KEY_MECHANISM         QLatin1String("Mechanism")

#define SSOUI_KEY_SLOT_ACCEPT  "accept"
#define SSOUI_KEY_SLOT_REJECT  "reject"
#define SSOUI_KEY_SLOT_REFRESH "refresh"

#define SSOUI_KEY_STORAGE_KEYS_UNAVAILABLE \
    QLatin1String("SecStorageKeysUnavailable")

#define SSOUI_KEY_UNABLE_TO_QUERY_STORAGE_KEYS \
    QLatin1String("SecStorageUnableQueryKeys")

#endif /* UISESSIONDATA_PRIV_H */
