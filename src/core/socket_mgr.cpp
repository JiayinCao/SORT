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

#include <cstring>
#include "socket_mgr.h"
#include "core/log.h"

void    SocketManager::Initialize() {
#ifdef SORT_IN_WINDOWS
    WSADATA wsaData;
    int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (err != NO_ERROR){
        m_initialized = false;
        slog(WARNING, SOCKET, "Failed to initialize socket manager.");
    }else {
        m_initialized = true;
    }
#else
    m_initialized = true;
#endif
}

//! @brief  Shutdown the socket manager.
void    SocketManager::Shutdown() {
#ifdef SORT_IN_WINDOWS
    WSACleanup();
#endif

    m_initialized = false;
}

socket_t SocketManager::AddSocket(const SOCKET_TYPE type, const std::string ip, const std::string& port) {
    socket_t sock = INVALID_SOCKET;
    
    // construct the socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        slog(WARNING, SOCKET, "Socket creation error.");
        return sock;
    }

    sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port.c_str()));

    if (auto err = inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0){
        slog(WARNING, SOCKET, "Invalid address/ Address not supported.");
        return sock;
    }

    if (auto err = connect(sock, (struct sockaddr*) & serv_addr, sizeof(serv_addr)) < 0){
        slog(WARNING, SOCKET, "Connection failed.");
        return sock;
    }

    slog(INFO, SOCKET, "Connected to socket:\t%s:%s", ip.c_str(), port.c_str());

    m_sockets.insert(sock);
    
    return sock;
}