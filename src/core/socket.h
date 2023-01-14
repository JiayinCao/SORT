/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.

    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#pragma once

#include "core/define.h"

#ifdef SORT_IN_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Ws2tcpip.h>
#include <winsock2.h>
#undef NOMINMAX
using socket_t = SOCKET;
#else
#define INVALID_SOCKET (-1)
using socket_t = int;
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

void InitializeSocketSystem();
void ShutdownSocketSystem();

void DisconnectSocket(struct SocketConnection* ptr);

struct SocketConnection {
    socket_t    m_socket = INVALID_SOCKET;
    bool        m_is_connected = false;

    ~SocketConnection() {
        if (!m_is_connected)
            return;
        DisconnectSocket(this);
    }
};

std::unique_ptr<SocketConnection> ConnectSocket(const std::string& ip, const std::string& port);