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

#include "abstract-crypto-manager.h"
#include "debug.h"
#include "key-handler.h"
#include <QSet>

using namespace SignOn;

namespace SignOn {

class KeyHandlerPrivate: public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KeyHandler)

public:
    KeyHandlerPrivate(KeyHandler *handler):
        q_ptr(handler),
        m_cryptoManager(0)
    {
    }
    ~KeyHandlerPrivate() {}

    void initialize(AbstractCryptoManager *cryptoManager,
                    const KeyManagersList &keyManagers);

    bool isReady() const
    {
        return m_readyKeyManagers.count() == m_keyManagers.count();
    }

    QSet<SignOn::Key> authorizedInsertedKeys() const
    {
        QSet<SignOn::Key> temp = m_insertedKeys;
        return temp.intersect(m_authorizedKeys);
    }

    const SignOn::Key &anyAuthorizedKey() const
    {
        return *m_authorizedKeys.constBegin();
    }

    bool authorizeKey(const SignOn::Key &key, KeyHandler::AuthorizeFlags flags);
    bool revokeKeyAuthorization(const SignOn::Key &key);

private Q_SLOTS:
    void onKeyInserted(const SignOn::Key key);
    void onKeyDisabled(const SignOn::Key key);
    void onKeyRemoved(const SignOn::Key key);

private:
    mutable KeyHandler *q_ptr;
    AbstractCryptoManager *m_cryptoManager;
    KeyManagersList m_keyManagers;
    KeyManagersList m_readyKeyManagers;
    QSet<SignOn::Key> m_insertedKeys;
    QSet<SignOn::Key> m_authorizedKeys;
};
};

void KeyHandlerPrivate::initialize(AbstractCryptoManager *cryptoManager,
                                   const KeyManagersList &keyManagers)
{
    m_cryptoManager = cryptoManager;
    m_keyManagers = keyManagers;

    if (keyManagers.isEmpty()) {
        TRACE() << "No key manager has been registered";
    }

    foreach (SignOn::AbstractKeyManager *keyManager, m_keyManagers) {
        connect(keyManager,
                SIGNAL(keyInserted(const SignOn::Key)),
                SLOT(onKeyInserted(const SignOn::Key)));
        connect(keyManager,
                SIGNAL(keyDisabled(const SignOn::Key)),
                SLOT(onKeyDisabled(const SignOn::Key)));
        connect(keyManager,
                SIGNAL(keyRemoved(const SignOn::Key)),
                SLOT(onKeyRemoved(const SignOn::Key)));
        keyManager->setup();
    }
}

bool KeyHandlerPrivate::authorizeKey(const SignOn::Key &key,
                                     KeyHandler::AuthorizeFlags flags)
{
    Q_Q(KeyHandler);

    if (m_authorizedKeys.contains(key)) {
        BLAME() << "Key already authorized";
        return true;
    }

    if (!m_cryptoManager->fileSystemIsSetup() ||
        flags & KeyHandler::FormatStorage) {
        m_authorizedKeys.clear();
        m_cryptoManager->setEncryptionKey(key);
        if (!m_cryptoManager->setupFileSystem()) {
            BLAME() << "Failed to setup the encrypted partition";
            return false;
        }
    } else { // FS is created and we must not reformat it
        if (m_authorizedKeys.isEmpty()) {
            BLAME() << "No authorized keys: cannot add new key";
            return false;
        }

        SignOn::Key authorizedKey = anyAuthorizedKey();
        if (!m_cryptoManager->fileSystemIsMounted()) {
            m_cryptoManager->setEncryptionKey(authorizedKey);
            if (!m_cryptoManager->mountFileSystem()) {
                BLAME() << "Couldn't mount FS: cannot add new key";
                return false;
            }
        }

        // at this point, the FS is mounted
        if (!m_cryptoManager->addEncryptionKey(key, authorizedKey)) {
            BLAME() << "Couldn't add new key";
            return false;
        }
    }

    m_authorizedKeys.insert(key);
    emit q->keyAuthorized(key);
    return true;
}

bool KeyHandlerPrivate::revokeKeyAuthorization(const SignOn::Key &key)
{
    Q_Q(KeyHandler);

    if (!m_cryptoManager->fileSystemIsSetup()) {
        TRACE() << "File system is not setup";
        return false;
    }

    if (m_authorizedKeys.isEmpty()) {
        BLAME() << "Cannot remove key: no authorized keys";
        return false;
    }

    SignOn::Key authorizedKey = anyAuthorizedKey();
    if (!m_cryptoManager->fileSystemIsMounted()) {
        m_cryptoManager->setEncryptionKey(authorizedKey);
        if (!m_cryptoManager->mountFileSystem()) {
            BLAME() << "Couldn't mount FS: cannot remove key";
            return false;
        }
    }

    // at this point, the FS is mounted; remove the key
    if (!m_cryptoManager->removeEncryptionKey(key, authorizedKey)) {
        BLAME() << "Failed to remove key";
        return false;
    }

    TRACE() << "Key successfully removed";
    m_authorizedKeys.remove(key);
    emit q->keyAuthorizationRevoked(key);
    return true;
}

void KeyHandlerPrivate::onKeyInserted(const SignOn::Key key)
{
    Q_Q(KeyHandler);

    TRACE() << "Key inserted.";

    if (!isReady()) {
        SignOn::AbstractKeyManager *manager =
            qobject_cast<SignOn::AbstractKeyManager *>(sender());
        if (!m_readyKeyManagers.contains(manager)) {
            m_readyKeyManagers.append(manager);
            if (isReady())
                emit q->ready();
        }
    }

    if (key.isEmpty()) return;

    m_insertedKeys.insert(key);

    if (m_cryptoManager->fileSystemIsSetup()) {
        /* The `key in use` check will attempt to mount using the new key if
           the file system is not already mounted
           */
        if (m_cryptoManager->encryptionKeyInUse(key)) {
            TRACE() << "Key already in use.";
            if (!m_authorizedKeys.contains(key))
                m_authorizedKeys.insert(key);
        }
    }

    emit q->keyInserted(key);
}


void KeyHandlerPrivate::onKeyDisabled(const SignOn::Key key)
{
    Q_Q(KeyHandler);

    TRACE() << "Key disabled.";

    emit q->keyDisabled(key);

    m_insertedKeys.remove(key);

    /* If no authorized inserted keys left, emit a special notification */
    if (authorizedInsertedKeys().isEmpty() &&
        m_authorizedKeys.contains(key)) {
        emit q->lastAuthorizedKeyRemoved(key);
    }
}

void KeyHandlerPrivate::onKeyRemoved(const SignOn::Key key)
{
    Q_Q(KeyHandler);

    TRACE() << "Key removed.";

    // Make sure the key is disabled:
    onKeyDisabled(key);

    q->keyRemoved(key);
}

KeyHandler::KeyHandler(QObject *parent):
    QObject(parent),
    d_ptr(new KeyHandlerPrivate(this))
{
}

KeyHandler::~KeyHandler()
{
    delete d_ptr;
}

void KeyHandler::initialize(AbstractCryptoManager *cryptoManager,
                            const KeyManagersList &keyManagers)
{
    Q_D(KeyHandler);
    d->initialize(cryptoManager, keyManagers);
}

AbstractCryptoManager *KeyHandler::cryptoManager() const
{
    Q_D(const KeyHandler);
    return d->m_cryptoManager;
}

bool KeyHandler::isReady() const
{
    Q_D(const KeyHandler);
    return d->isReady();
}

QSet<SignOn::Key> KeyHandler::insertedKeys() const
{
    Q_D(const KeyHandler);
    return d->m_insertedKeys;
}

bool KeyHandler::keyIsAuthorized(const SignOn::Key &key) const
{
    Q_D(const KeyHandler);
    return d->m_authorizedKeys.contains(key);
}

bool KeyHandler::authorizeKey(const SignOn::Key &key, AuthorizeFlags flags)
{
    Q_D(KeyHandler);
    return d->authorizeKey(key, flags);
}

bool KeyHandler::revokeKeyAuthorization(const SignOn::Key &key)
{
    Q_D(KeyHandler);
    return d->revokeKeyAuthorization(key);
}

bool KeyHandler::canAddKeyAuthorization() const
{
    Q_D(const KeyHandler);
    return !d->m_authorizedKeys.isEmpty();
}

#include "key-handler.moc"

