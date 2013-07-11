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

#ifndef TESTSINGLESIGNON_H
#define TESTSINGLESIGNON_H

#include <QObject>

class QProcess;

class TestSingleSignon: public QObject
{
    Q_OBJECT

private Q_SLOTS:

    /*
     * Start the signon daemon
     */
    void initTestCase();

    /*
     * End the signon daemon
     */
    void cleanupTestCase();

    /*
     * Test cases
     */
    void singleTestClient();
    void multipleClientsAtOnceTest();

private:
    void multipleClientsAtOnceContiousTest();
    void stressTest();

private:
    int m_numberOfTestClients;
};

#endif // TESTSINGLESIGNON_H
