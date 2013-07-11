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
/*!
 * @copyright Copyright (C) 2011 Nokia Corporation.
 * @license LGPL
 */

#ifndef SIGNON_PLUGINS_COMMON_IPC_H
#define SIGNON_PLUGINS_COMMON_IPC_H

enum PluginOperation {
    PLUGIN_OP_TYPE = 1,
    PLUGIN_OP_MECHANISMS,
    PLUGIN_OP_PROCESS,
    PLUGIN_OP_PROCESS_UI,
    PLUGIN_OP_REFRESH,
    PLUGIN_OP_CANCEL,
    PLUGIN_OP_STOP,
    PLUGIN_OP_LAST
};

enum PluginResponse {
    PLUGIN_RESPONSE_RESULT = 1,
    PLUGIN_RESPONSE_STORE,
    PLUGIN_RESPONSE_ERROR,
    PLUGIN_RESPONSE_SIGNAL,
    PLUGIN_RESPONSE_UI,
    PLUGIN_RESPONSE_REFRESHED,
    PLUGIN_RESPONSE_LAST
};

#endif // SIGNON_PLUGINS_COMMON_IPC_H
