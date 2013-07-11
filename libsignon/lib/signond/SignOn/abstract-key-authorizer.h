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

#ifndef SIGNON_ABSTRACT_KEY_AUTHORIZER_H
#define SIGNON_ABSTRACT_KEY_AUTHORIZER_H

#include "SignOn/abstract-key-manager.h"
#include "SignOn/key-handler.h"

#include <QObject>

namespace SignOn {

class AbstractKeyAuthorizerPrivate;

/*!
 * @class AbstractKeyAuthorizer
 * @brief Object holding the keys to the secure storage
 *
 * The AbstractKeyAuthorizer is where all keys to the secure storage are held.
 * It provides signals to notify when keys are inserted, authorized, removed,
 * disabled, as well as methods to authorize and unauthorize keys.
 */
class SIGNON_EXPORT AbstractKeyAuthorizer: public QObject
{
    Q_OBJECT

public:
    enum Result {
        Denied = 0, /*!< Key should not be added to the keyring */
        Approved,   /*!< Key can be added to the keyring */
        Exclusive,  /*!< Key can be set as the only key in the ring */
    };

    /*!
     * @enum Reason
     * Flags specifying why signond is asking for authorizing a key.
     * @sa queryKeyAuthorization
     */
    enum Reasons {
        SystemStarted     = 1 << 0,
        KeyInserted       = 1 << 1,
        StorageNeeded     = 1 << 2,
    };
    Q_DECLARE_FLAGS(Reason, Reasons);

    /*!
     * Constructor
     */
    explicit AbstractKeyAuthorizer(KeyHandler *keyHandler,
                                   QObject *parent = 0);

    /*!
     * Destructor
     */
    virtual ~AbstractKeyAuthorizer();

    /*!
     * @returns the KeyHandler object.
     */
    KeyHandler *keyHandler() const;

    /*!
     * This method is called by signond when a decision needs to be taken
     * on whether @key should be added to the list of keys authorized to
     * access the secure storage.
     * The result is delivered asynchronously through the
     * keyAuthorizationQueried() signal.
     * @note This method doesn't effectively add @key to the secure storage
     * keyring; it simply asks whether @key should be authorized or not.
     *
     * @param key The key for which authorization is asked.
     * @sa keyAuthorizationQueried()
     */
    virtual void queryKeyAuthorization(const SignOn::Key &key, Reason reason);

Q_SIGNALS:
    /*!
     * Emitted to deliver an asynchronous response to the
     * queryKeyAuthorization() method.
     * @param key The key whose authorization has been queried.
     * @param result The result of the query, as specified by the Result enum.
     */
    void keyAuthorizationQueried(const SignOn::Key key, int result);

private:
    AbstractKeyAuthorizerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(AbstractKeyAuthorizer)
};

} // namespace

#endif // SIGNON_ABSTRACT_KEY_AUTHORIZER_H
