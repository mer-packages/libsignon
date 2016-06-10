/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2011 Nokia Corporation.
 *
 * Contact: Aurel Popirtac <ext-aurel.popirtac@nokia.com>
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

#include "blobiohandler.h"

#include <QDBusArgument>
#include <QBuffer>
#include <QDataStream>
#include <QDebug>

#include "SignOn/signonplugincommon.h"

#define SIGNON_IPC_BUFFER_PAGE_SIZE 16384

using namespace SignOn;

BlobIOHandler::BlobIOHandler(QIODevice *readChannel,
                             QIODevice *writeChannel,
                             QObject *parent):
    QObject(parent),
    m_readChannel(readChannel),
    m_writeChannel(writeChannel),
    m_readNotifier(0),
    m_blobSize(-1)
{
}

void BlobIOHandler::setReadChannelSocketNotifier(QSocketNotifier *notifier)
{
    if (notifier == 0)
        return;

    m_readNotifier = notifier;
}

bool BlobIOHandler::sendData(const QVariantMap &map)
{
    if (m_writeChannel == 0) {
        TRACE() << "NULL write channel.";
        return false;
    }

    QDataStream stream(m_writeChannel);
    QByteArray ba = variantMapToByteArray(map);
    stream << ba.size();

    QVector<QByteArray> pages = pageByteArray(ba);
    for (int i = 0; i < pages.count(); ++i)
        stream << pages[i];

    return true;
}

void BlobIOHandler::setReadNotificationEnabled(bool enabled)
{
    if (enabled) {
        if (m_readNotifier != 0) {
            m_readNotifier->setEnabled(true);
            connect(m_readNotifier, SIGNAL(activated(int)),
                    this, SLOT(readBlob()));
        } else {
            connect(m_readChannel, SIGNAL(readyRead()),
                    this, SLOT(readBlob()));
        }
    } else {
        if (m_readNotifier != 0) {
            disconnect(m_readNotifier, SIGNAL(activated(int)),
                       this, SLOT(readBlob()));
            m_readNotifier->setEnabled(false);
        } else {
            disconnect(m_readChannel, SIGNAL(readyRead()),
                       this, SLOT(readBlob()));
        }
    }
}

void BlobIOHandler::receiveData(int expectedDataSize)
{
    m_blobBuffer.clear();
    m_blobSize = expectedDataSize;

    //Enable read notification only if more than 1 BLOB page is to be received
    //This does not allow duplicate read attempts if only 1 page is available
    if (m_blobSize > SIGNON_IPC_BUFFER_PAGE_SIZE)
        setReadNotificationEnabled(true);

    readBlob();
}

void BlobIOHandler::readBlob()
{
    QDataStream in(m_readChannel);

    QByteArray fractionBa;
    in >> fractionBa;
    m_blobBuffer.append(fractionBa);

    //Avoid infinite loops if the other party behaves badly
    if ((fractionBa.size() == 0) && (m_blobBuffer.size() < m_blobSize)) {
        setReadNotificationEnabled(false);
        emit error();
        return;
    }

    if (m_blobBuffer.size() == m_blobSize) {
        QVariantMap sessionDataMap;
        sessionDataMap = byteArrayToVariantMap(m_blobBuffer);

        if (m_blobSize > SIGNON_IPC_BUFFER_PAGE_SIZE)
            setReadNotificationEnabled(false);

        emit dataReceived(sessionDataMap);
    }
}

QVariantMap expandDBusArgumentValue(const QVariant &value, bool *success)
{
    // first, convert the QDBusArgument to a map
    QDBusArgument dbusValue = value.value<QDBusArgument>();
    QVariantMap converted;
    if (dbusValue.currentType() == QDBusArgument::MapType &&
        // We only care about a{sv}
        dbusValue.currentSignature() == "a{sv}") {
        converted = qdbus_cast<QVariantMap>(dbusValue);
    } else {
        *success = false;
        return QVariantMap();
    }

    // Then, check each value of the converted map
    // and if any QDBusArgument is a value, convert that.
    QVariantMap returnValue;
    QVariantMap::const_iterator i;
    for (i = converted.constBegin(); i != converted.constEnd(); ++i) {
        if (qstrcmp(i.value().typeName(), "QDBusArgument") == 0) {
            QVariantMap convertedValue = expandDBusArgumentValue(i.value(), success);
            if (*success == false) {
                //bail out to prevent error in serialization
                return QVariantMap();
            }
            returnValue.insert(i.key(), convertedValue);
        } else {
            returnValue.insert(i.key(), i.value());
        }
    }

    return returnValue;
}

static QVariantMap filterOutComplexTypes(const QVariantMap &map)
{
    QVariantMap filteredMap;
    QVariantMap::const_iterator i;
    for (i = map.constBegin(); i != map.constEnd(); i++) {
        if (qstrcmp(i.value().typeName(), "QDBusArgument") == 0) {
            bool success = true;
            QVariantMap convertedMap = expandDBusArgumentValue(i.value(), &success);
            if (success == false) {
                /* QDBusArgument are complex types; there is no QDataStream
                 * serialization for them, so keeping them in the map would
                 * make the serialization fail for the whole map, if we are
                 * unable to convert to a QVariantMap.
                 * Therefore, skip them. */
                BLAME() << "Found non-map QDBusArgument in data; skipping.";
                continue;
            }
            filteredMap.insert(i.key(), convertedMap);
        } else {
            filteredMap.insert(i.key(), i.value());
        }
    }
    return filteredMap;
}

QByteArray BlobIOHandler::variantMapToByteArray(const QVariantMap &map)
{
    QBuffer buffer;
    if (!buffer.open(QIODevice::WriteOnly))
        BLAME() << "Buffer opening failed.";

    QDataStream stream(&buffer);
    stream << filterOutComplexTypes(map);
    buffer.close();

    return buffer.data();
}

QVariantMap BlobIOHandler::byteArrayToVariantMap(const QByteArray &array)
{
    QByteArray nonConst = array;
    QBuffer buffer(&nonConst);
    if (!buffer.open(QIODevice::ReadOnly))
        BLAME() << "Buffer opening failed.";

    buffer.reset();
    QDataStream stream(&buffer);
    QVariantMap map;
    stream >> map;
    buffer.close();

    return map;
}

QVector<QByteArray> BlobIOHandler::pageByteArray(const QByteArray &array)
{
    QVector<QByteArray> dataPages;
    QByteArray ba = array;
    QBuffer pagingBuffer(&ba);

    if (!pagingBuffer.open(QIODevice::ReadOnly))
        BLAME() << "Error while paging BLOB. Buffer opening failed.";

    while (!pagingBuffer.atEnd()) {
        QByteArray page = pagingBuffer.read(SIGNON_IPC_BUFFER_PAGE_SIZE);
        dataPages.append(page);
    }
    pagingBuffer.close();

    return dataPages;
}
