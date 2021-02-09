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

#include <queue>
#include "core/thread.h"
#include "core/singleton.h"
#include "stream/sstream.h"

//! @brief  Display item
struct DisplayItemBase {
public:
    ~DisplayItemBase() {}

    virtual void Process(std::unique_ptr<OSocketStream>& streams) = 0;

    std::string     title;
    int             w, h;   // the size of the tile
    bool            is_blender_mode;
};

//! @brief  A tile for displaying
struct DisplayTile : public DisplayItemBase {
    int x, y;   // the position of the tile
    
    // data of the tile per channel, we have R/G/B
    std::unique_ptr<float[]>     m_data[SPECTRUM_SAMPLE];

    void Process(std::unique_ptr<OSocketStream>& stream) override;
};

//! @brief  New image to display
struct DisplayImageInfo : public DisplayItemBase {
    void Process(std::unique_ptr<OSocketStream>& stream) override;
};

//! @brief  Indicate that we are done
struct TerminateIndicator : public DisplayItemBase {
    void Process(std::unique_ptr<OSocketStream>& stream) override;
};

//! @brief  Full target update
struct FullTargetUpdate : public DisplayItemBase {
    void Process(std::unique_ptr<OSocketStream>& stream) override;
};

enum SocketStatus {
    UNINITIALIZE = 0,
    CONNECTION_FAILED,
    CONNECTION_SUCCEED
};

//! @brief  Display is responsible for displaying intermediate result of the ray traced images.
/*
 * Dynamic displaying intermediate buffer should work in either Blender or Tev(https://github.com/Tom94/tev).
 * Basically, it tries to establish socket connections with the servers and display the intermediate result if needed.
 * Eventually, both of Blender and tev should have similar logic of displaying results. Since tev is open-source and I
 * don't want to make any changes to their code base, I would make my SORT Blender plugin works in a similar way so that
 * SORT doesn't need to differentiate the server type. Of course, if somehow in the future I need to do so, it could
 * easily be done later.
 */
class DisplayManager : public Singleton<DisplayManager> {
public:
    //! @brief  Add a display server in the manager.
    //!
    //! It is possible to add multiple servers at a same time so that all display servers get responses.
    //!
    //! @param host         The ip address of the server.
    //! @param port         The port the server is listening.
    void AddDisplayServer(const std::string host, const std::string& port);

    //! @brief  Whether there is any display server connected
    bool IsDisplayServerConnected() const;

    //! @brief  Resolve connection
    void ResolveDisplayServerConnection();

    //! @brief  Refresh the tile in display servers
    //!
    //! @param item         A display item to process
    void QueueDisplayItem(std::shared_ptr<DisplayItemBase> item);

    //! @brief  Update display
    //!
    //! @param  cnt         Number of display items to handle at a time, 4 by default. If -1 is set, there is no limitation.
    void ProcessDisplayQueue(int cnt = 0xffffffff);

private:
    /**< This data structure keeps track of all streams of servers. */
    std::unique_ptr<OSocketStream>                  m_stream;

    /**< Display queue. **/
    std::queue<std::shared_ptr<DisplayItemBase>>    m_queue;
    /**< A spin lock to protect the queue from being accessed in multiple threads. */
    spinlock_mutex                                  m_lock;

    /**< Whether the server is connected. */
    SocketStatus                                    m_status = SocketStatus::UNINITIALIZE;
    /**< The socket of the server. */
    socket_t                                        m_socket;
};