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

#ifndef BLOBIOHANDLER_H
#define BLOBIOHANDLER_H

#include <QObject>
#include <QIODevice>
#include <QVariantMap>
#include <QSocketNotifier>

namespace SignOn {

class BlobIOHandler: public QObject
{
    Q_OBJECT

public:
    BlobIOHandler(QIODevice *inputChannel,
                  QIODevice *outputChannel,
                  QObject *parent = 0);
    //sync call
    bool sendData(const QVariantMap &map);
    //async call
    void receiveData(int expectedDataSize);

    void setReadChannelSocketNotifier(QSocketNotifier *notifier);

public Q_SLOTS:
    void readBlob();

Q_SIGNALS:
    void dataReceived(const QVariantMap &map);
    void error();

private:
    void setReadNotificationEnabled(bool enable);

    QByteArray variantMapToByteArray(const QVariantMap &map);
    QVariantMap byteArrayToVariantMap(const QByteArray &array);
    QVector<QByteArray> pageByteArray(const QByteArray &array);

public:
    QIODevice *m_readChannel;
    QIODevice *m_writeChannel;
    QByteArray m_blobBuffer;
    QSocketNotifier *m_readNotifier;
    int m_blobSize;
};

}

#endif //BLOBIOHANDLER_H
