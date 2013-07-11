/*
 * This file is part of signon
 *
 * Copyright (C) 2010 Nokia Corporation.
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
/*!
 * @copyright Copyright (C) 2009-2011 Nokia Corporation.
 * @license LGPL
 */

#ifndef SIGNON_ABSTRACT_KEY_MANAGER_H
#define SIGNON_ABSTRACT_KEY_MANAGER_H

#include <SignOn/extension-interface.h>

#include <QObject>
#include <QString>

namespace SignOn {

class AbstractKeyManagerPrivate;

typedef QByteArray Key;

/*!
 * @class AbstractKeyManager
 * @brief Base class for KeyManager implementations.
 *
 * The AbstractKeyManager is the base class for implementing signond key
 * managers. A key manager is an object which signond uses to get the keys
 * needed to access the encrypted storage. It emits signals whenever new keys
 * exist, when keys are disabled or removed. For example, a simple password
 * based key manager would:
 * @li query the user for the password when queryKeys() is called.
 * @li emit keyInserted() whenever the user enters the password.
 * @li if the password automatically expires after some time, emit keyDisabled
 * @li if the user changes the password, emit keyInserted() with the new
 * passord, and keyRemoved() for the old one.
 */
class SIGNON_EXPORT AbstractKeyManager: public QObject
{
    Q_OBJECT

public:
    /*!
     * Constructor
     */
    explicit AbstractKeyManager(QObject *parent = 0);

    /*!
     * Destructor
     */
    virtual ~AbstractKeyManager();

    /*!
     * This method initializes the key manager. Implementations can start
     * emitting signals during this method's execution.
     */
    virtual void setup() = 0;

    /*!
     * Signond calls this method if a key (typically a new key emitted with
     * the keyInserted() signal) needs to be authorized. Authorizing a key
     * means allowing the key to be used to mount the secure storage. The base
     * implementation of this method just emits the keyAuthorized() signal
     * denying the authorization, but specific implementations could delegate
     * the authorization to the user.
     * @param key The key that needs authorization.
     * @param message An optional message that might be shown to the user.
     *
     * @deprecated This method is deprecated and should not be
     * used or implemented in subclasses.
     */
    virtual void authorizeKey(const Key &key,
                              const QString &message = QString::null);

    /*!
     * Signond calls this method when there are no active keys. This might
     * happen when signond is just starting, or when all existing keys have
     * been disabled (which would cause the secure storage to be unmounted).
     * The key manager <em>must</em> emit keyInserted() in response to this
     * call, either with a valid key or with an empty one.
     */
    virtual void queryKeys();

    /*!
     * Gets a description for the key.
     * @param key The key whose name is to be returned
     * @return A description or name that could be shown to the user to
     * identify the key
     */
    virtual QString describeKey(const Key &key);

    /*!
     * @returns Whether the extension is able to authorize keys or not.
     * The default implementation just returns false.
     *
     * @deprecated This method is deprecated and should not be
     * used or implemented in subclasses.
     */
    virtual bool supportsKeyAuthorization() const;

Q_SIGNALS:
    /*!
     * Emitted when a new key is available. If the key is not yet known to
     * signond, signond will call authorizeKey on the key managers before
     * accepting it.
     * @param key The new key
     */
    void keyInserted(const SignOn::Key key);

    /*!
     * Emitted when a key is disabled. For instance, this signal can be
     * emitted when a password expires or when a hardware key is removed.
     * @param key The key which has been disabled
     */
    void keyDisabled(const SignOn::Key key);

    /*!
     * Emitted when a key is to be removed. This means that the key will never
     * be used again to access the secure storage; if it will be inserted
     * again, it will have to be re-authenticated.
     * It is not necessary to emit keyDisabled() before keyRemoved(): the key
     * will be disabled anyway when keyRemoved is emitted.
     * @param key The key which has to be removed
     */
    void keyRemoved(const SignOn::Key key);

    /*!
     * The key manager emits this when it has decided whether to authorized
     * the key. If more than one key manager is installed, signond will
     * authorized the key as soon as the first key manager emits this signal
     * with authorized set to true.
     * @param key The key which underwent the authorization step
     * @param authorized The result of the authorization
     *
     * @deprecated This signal is deprecated and should not be
     * used or emitted in subclasses.
     */
    void keyAuthorized(const SignOn::Key key, bool authorized);

private:
    AbstractKeyManagerPrivate *d;
};

} // namespace

#endif // SIGNON_ABSTRACT_KEY_MANAGER_H
