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

#include "dbusoperationqueuehandler.h"

#include <QMetaMethod>
#include <QDebug>
#include <QMetaType>

#include "libsignoncommon.h"
#include "identityinfo.h"

/*
 * @cond IMPL
 */
namespace SignOn {

/* --------------- DBusOperationQueueHandler::Operation ---------------- */

DBusOperationQueueHandler::Operation::Operation(const char *name,
                                                QList<QGenericArgument *> args)
{
    copy(name, args);
    qDeleteAll(args);
}

void DBusOperationQueueHandler::Operation::copy(const char *name,
                                                const QList<QGenericArgument *> &args)
{
    Q_ASSERT(name != NULL);

    m_name = new char[qstrlen(name) + 1];
    qstrcpy(m_name, name);

    QListIterator<QGenericArgument *> it(args);
    while (it.hasNext()) {
        QGenericArgument *arg = it.next();
        int type = QMetaType::type(arg->name());
        if (!QMetaType::isRegistered(type)) {
            qCritical()
                << Q_FUNC_INFO
                << QString(QLatin1String("Type %1 not registered."))
                .arg(QLatin1String(arg->name()));
        } else {
            Q_ASSERT(arg->name() != NULL);

            char *localName = new char[qstrlen(arg->name()) + 1];
            qstrcpy(localName, arg->name());
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
            void *localData = QMetaType::construct(type, arg->data());
#else
            void *localData = QMetaType::create(type, arg->data());
#endif

            m_args << (new QGenericArgument(localName, localData));
        }
    }
}

DBusOperationQueueHandler::Operation::~Operation()
{
    if (m_name)
        delete [] m_name;

    foreach (QGenericArgument *arg, m_args) {
        QMetaType::destroy(QMetaType::type(arg->name()), arg->data());
        if (arg->name())
            delete [] arg->name();
        delete arg;
    }
}

/* --------------------- DBusOperationQueueHandler --------------------- */

DBusOperationQueueHandler::DBusOperationQueueHandler(QObject *clientObject):
    m_clientObject(clientObject),
    m_maxNumberOfOperationParameters(6),
    m_operationsStopped(false)
{
}

DBusOperationQueueHandler::~DBusOperationQueueHandler()
{
}

void DBusOperationQueueHandler::enqueueOperation(Operation *operation)
{
    m_operationsQueue.enqueue(operation);
}

void DBusOperationQueueHandler::enqueueOperation(const char *name,
                                                 QList<QGenericArgument *> args)
{
    m_operationsQueue.enqueue(new Operation(name, args));
}

void DBusOperationQueueHandler::execQueuedOperations()
{
    m_operationsStopped = false;

    while (m_operationsStopped == false && !m_operationsQueue.empty()) {
        Operation *op = m_operationsQueue.dequeue();

        if (op->m_args.size() > m_maxNumberOfOperationParameters) {
            qWarning() << "DBusOperationQueueHandler::execQueuedOperations(): "
                          "Maximum number of operation parameters exceeded(6).";
            continue;
        }

        int indexOfMethod = m_clientObject->metaObject()->indexOfMethod(
                                QMetaObject::normalizedSignature(op->m_name));

        QMetaMethod method = m_clientObject->metaObject()->method(indexOfMethod);

        TRACE() << "Executing cached oparation: SIGNATURE:" <<
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
            method.signature();
#else
            method.methodSignature();
#endif

        switch (op->m_args.count()) {
        case 0: TRACE(); method.invoke(m_clientObject); break;
        case 1: TRACE(); method.invoke(
                                    m_clientObject,
                                    *(op->m_args.at(0))); break;
        case 2: TRACE(); method.invoke(
                                    m_clientObject,
                                    *(op->m_args.at(0)),
                                    *(op->m_args.at(1))); break;
        case 3: TRACE(); method.invoke(
                                    m_clientObject,
                                    *(op->m_args.at(0)),
                                    *(op->m_args.at(1)),
                                    *(op->m_args.at(2))); break;
        case 4: TRACE(); method.invoke(
                                    m_clientObject,
                                    *(op->m_args.at(0)),
                                    *(op->m_args.at(1)),
                                    *(op->m_args.at(2)),
                                    *(op->m_args.at(3))); break;
        case 5: TRACE(); method.invoke(
                                    m_clientObject,
                                    *(op->m_args.at(0)),
                                    *(op->m_args.at(1)),
                                    *(op->m_args.at(2)),
                                    *(op->m_args.at(3)),
                                    *(op->m_args.at(4))); break;
        case 6: TRACE(); method.invoke(
                                    m_clientObject,
                                    *(op->m_args.at(0)),
                                    *(op->m_args.at(1)),
                                    *(op->m_args.at(2)),
                                    *(op->m_args.at(3)),
                                    *(op->m_args.at(4)),
                                    *(op->m_args.at(5))); break;
        default: TRACE(); method.invoke(m_clientObject); break;
        }
        delete op;
    }
}

void DBusOperationQueueHandler::removeOperation(const char *name, bool removeAll)
{
    foreach (Operation *operation, m_operationsQueue) {
        if (operation != NULL && qstrcmp(operation->m_name, name) == 0) {
            m_operationsQueue.removeOne(operation);
            if (!removeAll)
                break;
        }
    }
}

bool DBusOperationQueueHandler::queueContainsOperation(const char *name)
{
    foreach (Operation *operation, m_operationsQueue)
        if (operation != NULL && qstrcmp(operation->m_name, name) == 0)
            return true;

    return false;
}

} //SignOn
/*
 * @endcond IMPL
 */
