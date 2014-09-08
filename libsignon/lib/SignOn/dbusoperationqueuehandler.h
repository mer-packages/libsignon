/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
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

#ifndef DBUSOPERATIONQUEUEHANDLER_H
#define DBUSOPERATIONQUEUEHANDLER_H

#include <QQueue>
#include <QDBusInterface>


#define SIGNOND_NORMALIZE_METHOD_SIGNATURE(method) \
    DBusOperationQueueHandler::normalizedOperationSignature(method).data()

/*
 * @cond IMPL
 */
namespace SignOn {

class DBusOperationQueueHandler
{
public:
    struct Operation
    {
        Operation(const char *name,
                  QList<QGenericArgument *> args = QList<QGenericArgument *>());
        ~Operation();

        inline bool operator==(const Operation &op) const
            { return qstrcmp(op.m_name, m_name) == 0; }

        char *m_name;
        QList<QGenericArgument *> m_args;

    private:
        void copy(const char *name,
                  const QList<QGenericArgument *> &args);
    };

public:
    DBusOperationQueueHandler(QObject *clientObject);
    ~DBusOperationQueueHandler();

    void enqueueOperation(Operation *operation);
    void enqueueOperation(const char *name,
                          QList<QGenericArgument *> args = QList<QGenericArgument *>());

    void execQueuedOperations();
    int queuedOperationsCount() const { return m_operationsQueue.count(); }
    void clearOperationsQueue();

    void removeOperation(const char *name, bool removeAll = true);

    bool queueContainsOperation(const char *name);
    void stopOperationsProcessing() { m_operationsStopped = true; }

    static QByteArray normalizedOperationSignature(const char *operationName)
        { return QMetaObject::normalizedSignature(operationName); }

private:
    QObject *m_clientObject;
    const int m_maxNumberOfOperationParameters;
    QQueue<Operation *> m_operationsQueue;
    bool m_operationsStopped;
};

} //SignOn

/*
 * @endcond IMPL
 */

#endif // DBUSOPERATIONQUEUEHANDLER_H
