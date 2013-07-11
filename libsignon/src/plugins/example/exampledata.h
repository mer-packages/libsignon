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
#ifndef EXAMPLEDATA_H
#define EXAMPLEDATA_H

#include <SignOn/sessiondata.h>

/*!
 * @class ExampleData
 * Data container to hold values for example authentication session.
 */
class ExampleData: public SignOn::SessionData
{
public:
    /*!
     * Declare property Example setter and getter
     */
    SIGNON_SESSION_DECLARE_PROPERTY(QString, Example);

    /*!
     * Declare property Params setter and getter
     */
    SIGNON_SESSION_DECLARE_PROPERTY(QString, Params);

    /*!
     * Declare property Tos setter and getter
     * Can be used to set Terms of Service message
     */
    SIGNON_SESSION_DECLARE_PROPERTY(QString, Tos);
};

#endif // EXAMPLEDATA_H
