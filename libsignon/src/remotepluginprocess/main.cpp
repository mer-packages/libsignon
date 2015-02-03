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

extern "C" {
#include <signal.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <syslog.h>
}

#define QT_DISABLE_DEPRECATED_BEFORE QT_VERSION_CHECK(4, 0, 0)

#include "debug.h"
#include "remotepluginprocess.h"

#include <QDebug>

using namespace RemotePluginProcessNS;

RemotePluginProcess *process = NULL;

void messageHandler(QtMsgType type, const char *msg)
{
    if (debugLevel < 2) {
        if (type == QtDebugMsg) return;
        if (debugLevel < 1 && type == QtWarningMsg) return;
    }

    int priority;
    switch (type) {
    case QtWarningMsg: priority = LOG_WARNING; break;
    case QtCriticalMsg: priority = LOG_CRIT; break;
    case QtFatalMsg: priority = LOG_EMERG; break;
    case QtDebugMsg:
                     /* fall through */
    default: priority = LOG_INFO; break;
    }

    syslog(priority, "%s", msg);
}

int main(int argc, char *argv[])
{
    openlog(NULL, LOG_CONS | LOG_PID, LOG_DAEMON);
    qInstallMsgHandler(messageHandler);
    debugInit();

    TRACE() << "handler:" << (void *)messageHandler;

#ifndef NO_SIGNON_USER
    if (!::getuid()) {
        BLAME() << argv[0] << " cannot be started with root priviledges!!!";
        exit(2);
    }
#endif

    QCoreApplication app(argc, argv);

    if (argc < 2) {
        TRACE() << "Type of plugin is not specified";
        exit(1);
    }

    QString type = app.arguments().at(1); TRACE() << type;

    fcntl(fileno(stdin), F_SETFL, fcntl(fileno(stdin), F_GETFL, 0) | O_NONBLOCK);

    process = RemotePluginProcess::createRemotePluginProcess(type, &app);

    if (!process)
        return 1;

    fprintf(stdout, "process started");
    fflush(stdout);

    QObject::connect(process, SIGNAL(processStopped()), &app, SLOT(quit()));
    int ret = app.exec();
    closelog();
    return ret;
}
