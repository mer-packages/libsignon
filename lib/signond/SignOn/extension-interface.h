/*
 * This file is part of signon
 *
 * Copyright (C) 2010-2011 Nokia Corporation.
 * Copyright (C) 2011 Canonical Ltd.
 * Copyright (C) 2011 Intel Corporation.
 *
 * Contact: Alberto Mardegan <alberto.mardegan@canonical.com>
 * Contact: Jussi Laako <jussi.laako@linux.intel.com>
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
/*!
 * @copyright Copyright (C) 2009-2011 Nokia Corporation.
 * @license LGPL
 */

#ifndef SIGNON_EXTENSION_INTERFACE_H
#define SIGNON_EXTENSION_INTERFACE_H

#include <SignOn/export.h>

#include <QObject>
#include <QtPlugin>

namespace SignOn {

class AbstractCryptoManager;
class AbstractKeyAuthorizer;
class AbstractKeyManager;
class AbstractSecretsStorage;
class KeyHandler;
class AbstractAccessControlManager;

/*!
 * @class ExtensionInterface.
 * Interface definition for signond extensions.
 */
class SIGNON_EXPORT ExtensionInterface
{
public:
    virtual ~ExtensionInterface() {}

    /*!
     * Gets the KeyManager object.
     *
     * @return A key manager object, or 0 if none is provided by this plugin
     */
    virtual AbstractKeyManager *keyManager(QObject *parent = 0) const;
};

/*!
 * @class ExtensionInterface2.
 * Interface definition for signond extensions.
 */
class SIGNON_EXPORT ExtensionInterface2: public ExtensionInterface
{
public:
    virtual ~ExtensionInterface2() {}

    /*!
     * Gets the KeyAuthorizer object.
     *
     * @return A key authorizer object, or 0 if none is provided by this plugin
     */
    virtual AbstractKeyAuthorizer *keyAuthorizer(KeyHandler *keyHandler,
                                                 QObject *parent = 0) const;
};

/*!
 * @class ExtensionInterface3.
 * Interface definition for signond extensions.
 */
class SIGNON_EXPORT ExtensionInterface3: public ExtensionInterface2
{
public:
    virtual ~ExtensionInterface3() {}

    /*!
     * Gets the CryptoManager object, which will be used to setup the file
     * system for the credentials storage.
     * This object is instantiated only if the value of the "CryptoManager"
     * setting in the signond configuration matches the plugin's
     * QObject::objectName().
     *
     * @return A CryptoManager object, or 0 if none is provided by this plugin
     */
    virtual AbstractCryptoManager *cryptoManager(QObject *parent = 0) const;

    /*!
     * Gets the SecretsStorage object, which is used to save and load the
     * user's secrets, as well as authentication plugins' data.
     * This object is instantiated only if the value of the "SecretsStorage"
     * setting in the signond configuration matches the plugin's
     * QObject::objectName().
     *
     * @return A SecretsStorage object, or 0 if none is provided by this plugin.
     */
    virtual AbstractSecretsStorage *secretsStorage(QObject *parent = 0) const;

    /*!
     * Gets the AbstractAccessControlManager object, which will be used to
     * check accesses to the credential database.
     * This object is instantiated only if the value of the
     * "AccessControlManager" setting in the signond configuration matches the
     * plugin's QObject::objectName().
     *
     * @return An AbstractAccessControlManager object, or 0 if none is provided
     * by this plugin
     */
    virtual AbstractAccessControlManager *accessControlManager(
                                                    QObject *parent = 0) const;
};

} // namespace

Q_DECLARE_INTERFACE(SignOn::ExtensionInterface,
                    "com.nokia.SingleSignOn.ExtensionInterface/1.0")
Q_DECLARE_INTERFACE(SignOn::ExtensionInterface2,
                    "com.nokia.SingleSignOn.ExtensionInterface/2.0")
Q_DECLARE_INTERFACE(SignOn::ExtensionInterface3,
                    "com.nokia.SingleSignOn.ExtensionInterface/3.0")

#endif // SIGNON_EXTENSION_INTERFACE_H
