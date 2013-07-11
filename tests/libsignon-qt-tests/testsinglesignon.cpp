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

#include <QtTest/QtTest>
#include <QtCore>

#include <sys/types.h>
#include <errno.h>

#include <signal.h>

#include <signond/signoncommon.h>

#include "testsinglesignon.h"

#include "ssotestclient.h"
#include "ssoclientthread.h"

#include <QDBusConnection>

void TestSingleSignon::initTestCase()
{
    m_numberOfTestClients = NUMBER_OF_TEST_CLIENTS;
}

void TestSingleSignon::cleanupTestCase()
{
}

void TestSingleSignon::singleTestClient()
{
    QTime startTime = QTime::currentTime();

    SsoTestClient client;
    client.runAllTests();

    QTime endTime = QTime::currentTime();
    int elapsed = startTime.secsTo(endTime);
    qDebug() << QString("TestSingleSignon::singleTestClient() "
                        "---- TIME --> Total elapsed time: %1 seconds.").
        arg(elapsed);
}

void TestSingleSignon::multipleClientsAtOnceTest()
{
    QList<SsoClientThread *> clientThreads;

    QTime startTime = QTime::currentTime();
    for(int i = 0; i < m_numberOfTestClients; i++)
    {
        qDebug() << "Creating SSO test client thread...";
        SsoClientThread *thread = new SsoClientThread(SsoTestClient::AllTests);
        thread->start();
        clientThreads.append(thread);
    }

    SsoClientThread *thread = NULL;
    bool done = false;

    //this assumes all client threads will be finished at a certain point
    while(!done)
    {
        int finishedThreadsCount = 0;
        foreach(thread, clientThreads)
            if(thread->isFinished()) ++finishedThreadsCount;

        done = ((finishedThreadsCount == m_numberOfTestClients) ? true : false);
        sleep(1);
    }

    while(!clientThreads.empty())
        if(SsoClientThread *thread = clientThreads.takeFirst()) delete thread;

    QTime endTime = QTime::currentTime();
    int elapsed = startTime.secsTo(endTime);
    qDebug() << QString("TestSingleSignon::multipleClientsAtOnceTest() "
                        "---- TIME --> Total elapsed time: %1 seconds.").
        arg(elapsed);
}

void TestSingleSignon::multipleClientsAtOnceContiousTest()
{
    QTime startTime = QTime::currentTime();
    int rounds = 5;

    while(rounds > 0)
    {
        QList<SsoClientThread *> clientThreads;

        for (int i = 0; i < m_numberOfTestClients; i++)
        {
            qDebug() << "\n\nCreating SSO test client thread...";
            SsoClientThread *thread =
                new SsoClientThread(SsoTestClient::AllTests);
            thread->start();
            clientThreads.append(thread);
        }

        SsoClientThread *thread = NULL;
        bool done = false;
        //this assumes all client threads will be finished at a certain point
        while (!done)
        {
            int finishedThreadsCount = 0;
            foreach(thread, clientThreads)
                if (thread->isFinished()) ++finishedThreadsCount;

            done = ((finishedThreadsCount == m_numberOfTestClients) ?
                    true : false);
            sleep(1);
        }
        while (!clientThreads.empty())
            if (SsoClientThread *thread = clientThreads.takeFirst())
                delete thread;

        --rounds;
    }

    QTime endTime = QTime::currentTime();
    int elapsed = startTime.secsTo(endTime);
    qDebug() << QString("TestSingleSignon::multipleClientsAtOnceContiousTest() "
                        "---- TIME --> Total elapsed time: %1 seconds.").
        arg(elapsed);
}

void TestSingleSignon::stressTest()
{
    //think of something awful here
}

#ifndef SSO_CI_TESTMANAGEMENT
    QTEST_MAIN(TestSingleSignon)
#endif
