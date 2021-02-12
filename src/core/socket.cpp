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

#include "socket.h"
#include "core/log.h"

void InitializeSocketSystem() {
#ifdef SORT_IN_WINDOWS
    WSADATA wsaData;
    int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (err != NO_ERROR)
        slog(WARNING, SOCKET, "Failed to initialize socket manager.");
#endif
}

void ShutdownSocketSystem(){
#ifdef SORT_IN_WINDOWS
    WSACleanup();
#endif
}

std::unique_ptr<SocketConnection> ConnectSocket(const std::string& ip, const std::string& port) {
    auto ret = std::make_unique<SocketConnection>();

    ret->m_is_connected = false;

    // construct the socket
    if ((ret->m_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        slog(WARNING, SOCKET, "Socket creation error.");
        return nullptr;
    }

    sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port.c_str()));

    if (auto err = inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
        slog(WARNING, SOCKET, "Invalid address/ Address not supported.");
        return nullptr;
    }

    // try to connect now
    const auto status = connect(ret->m_socket, (struct sockaddr*) & serv_addr, sizeof(serv_addr));
    if (status < 0) {
        slog(WARNING, SOCKET, "Failed to connect to server.");
        return nullptr;
    }

    ret->m_is_connected = true;
    return ret;
}

void DisconnectSocket(SocketConnection* ptr) {
    if (!ptr)
        return;

#ifdef SORT_IN_WINDOWS
    closesocket(ptr->m_socket);
#else
    close(ptr->m_socket);
#endif
}