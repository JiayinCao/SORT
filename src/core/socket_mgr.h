/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.

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

#include <unordered_set>
#include "core/define.h"
#include "core/singleton.h"

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
#endif

enum SOCKET_TYPE {
    SERVER,
    CLIENT
};

//! @brief  A manager keep track of all sockets in the program.
class SocketManager : public Singleton<SocketManager> {
public:
    //! @brief  Initializing the socket manager.
    void    Initialize();

    //! @brief  Shutdown the socket manager.
    void    Shutdown();

    //! @brief  Add a socket.
    //!
    //! @param type         Whether we are server type socket or client type.
    //! @param host         The ip address of the server.
    //! @param port         The port the server is listening.
    socket_t AddSocket(const SOCKET_TYPE type, const std::string host, const std::string& port);

private:
    /**< This data structure keeps track of all sockets. */
    std::unordered_set<socket_t> m_sockets;

    /**< Whether the manager is initialized */
    bool m_initialized;
};