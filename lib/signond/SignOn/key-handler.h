/*
 * This file is part of signon
 *
 * Copyright (C) 2011 Nokia Corporation.
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
 * @copyright Copyright (C) 2011 Nokia Corporation.
 * @license LGPL
 */

#ifndef SIGNON_KEY_HANDLER_H
#define SIGNON_KEY_HANDLER_H

#include "SignOn/abstract-key-manager.h"

#include <QObject>
#include <QSet>

namespace SignOn {

class AbstractCryptoManager;

class KeyHandlerPrivate;
typedef QList<SignOn::AbstractKeyManager *> KeyManagersList;

/*!
 * @class KeyHandler
 * @brief Object holding the keys to the secure storage
 *
 * The KeyHandler is where all keys to the secure storage are held. It provides
 * signals to notify when keys are inserted, authorized, removed, disabled, as
 * well as methods to authorize and unauthorize keys.
 */
class SIGNON_EXPORT KeyHandler: public QObject
{
    Q_OBJECT

public:
    enum Authorizations {
        None = 0,
        FormatStorage = 1 << 0, /*!< Formats the secure storage */
    };
    Q_DECLARE_FLAGS(AuthorizeFlags, Authorizations);

    /*!
     * Constructor
     */
    explicit KeyHandler(QObject *parent = 0);

    /*!
     * Destructor
     */
    virtual ~KeyHandler();

    /*!
     * This method initializes the key managers.
     */
    void initialize(AbstractCryptoManager *cryptoManager,
                    const KeyManagersList &keyManagers);

    /*!
     * @returns The CryptoManager.
     */
    AbstractCryptoManager *cryptoManager() const;

    /*!
     * True if all key managers have been initialized and have started
     * reporting their inserted keys.
     */
    bool isReady() const;

    /*!
     * @returns the currently inserted keys.
     */
    QSet<SignOn::Key> insertedKeys() const;

    /*!
     * @param key The key to be tested.
     * @returns true if @key is an authorized key.
     */
    bool keyIsAuthorized(const SignOn::Key &key) const;

    /*!
     * Add @key to the list of keys which can be used to access the secure
     * storage. If the KeyHandler is empty, this method will fail.
     * @param key The key to be authorized.
     * @returns true if the operation was successful.
     * @sa keyAuthorized()
     */
    bool authorizeKey(const SignOn::Key &key, AuthorizeFlags flags = None);

    /*!
     * Remove @key from the list of keys which can be used to access the
     * secure storage. If the KeyHandler is empty, this method will fail.
     * @param key The key to be revoked.
     * @returns true if the operation was successful.
     * @sa keyAuthorizationRevoked()
     */
    bool revokeKeyAuthorization(const SignOn::Key &key);

    /*!
     * Returns true if a new key can be authorized by calling authorizeKey()
     * without specifying the FormatStorage flag (that is, if the KeyHandler
     * knows other keys which can open the storage).
     */
    bool canAddKeyAuthorization() const;

Q_SIGNALS:
    /*!
     * Emitted after all key managers have been initialized and have reported
     * about their inserted keys.
     */
    void ready();

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
     * This signal will be emitted before the action is processed by the
     * KeyHandler; this means that if for instance @key was an authorized key,
     * calling keyIsAuthorized() from this signal's handler will still return
     * true.
     * @param key The key which has been disabled
     */
    void keyDisabled(const SignOn::Key key);

    /*!
     * One of the key managers has asked that @key be removed. This means that
     * the key manager will never provide the same key again, so its
     * authorization to open the secure storage can be revoked.
     * The keyDisabled() signal will always be emitted prior to this one.
     * @param key The key which has been removed.
     */
    void keyRemoved(const SignOn::Key key);

    /*!
     * Emitted when a key authorization has been revoked.
     * @param key The key whose authorization was revoked.
     */
    void keyAuthorizationRevoked(const SignOn::Key key);

    /*!
     * Emitted when a key has been authorized.
     * @param key The key which was authorized.
     */
    void keyAuthorized(const SignOn::Key key);

    /*!
     * Emitted when the last authorized key has been removed.
     */
    void lastAuthorizedKeyRemoved(const SignOn::Key key);

private:
    KeyHandlerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(KeyHandler)
};

} // namespace

#endif // SIGNON_KEY_HANDLER_H
