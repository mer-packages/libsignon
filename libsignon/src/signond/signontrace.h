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
#ifndef SIGNONTRACE_H
#define SIGNONTRACE_H

#include <syslog.h>

#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QDateTime>

#include "signond-common.h"

namespace SignOn {

class SignonTrace
{
public:
    enum LogOutput {
        Syslog = 0,
        Stdout,
    };

    ~SignonTrace()
    {
        m_pInstance = NULL;
        if (m_logOutput == Syslog) {
            closelog();
        }
    }

    static void initialize(LogOutput logOutput)
    {
        if (m_pInstance)
            return;

        m_pInstance = new SignonTrace(logOutput);
    }

    static void output(QtMsgType type, const char *msg)
    {
        if (!m_pInstance)
            return;

        if (!criticalsEnabled()) {
            if (type <= QtCriticalMsg) return;
        } else if (!debugEnabled()) {
            if (type <= QtDebugMsg) return;
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

private:
    SignonTrace(LogOutput logOutput):
        m_logOutput(logOutput)
    {
        if (logOutput == Syslog) {
            openlog(NULL, LOG_PID, LOG_DAEMON);
            qInstallMsgHandler(output);
        }
    }

    static SignonTrace *m_pInstance;
    LogOutput m_logOutput;
};

SignonTrace *SignonTrace::m_pInstance = 0;

} //namespace SignOn

#endif // SIGNONTRACE_H
