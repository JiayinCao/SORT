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

#include <queue>
#include "core/thread.h"
#include "core/singleton.h"
#include "core/socket.h"
#include "stream/sstream.h"
#include "texture/rendertarget.h"

class RenderTarget;

//! @brief  Display item
struct DisplayItemBase {
public:
    DisplayItemBase(const std::string& title, const int w, const int h, const bool blender_mode)
        :title(title), w(w), h(h), is_blender_mode(blender_mode){}
    ~DisplayItemBase() {}

    virtual void Process(std::unique_ptr<OSocketStream>& streams) = 0;

protected:
    const std::string     title;
    const int             w, h;   // the size of the tile
    const bool            is_blender_mode;
};

//! @brief  A tile for displaying
struct DisplayTile : public DisplayItemBase {
    DisplayTile(const std::string& title, const int x, const int y, 
                const int w, const int h, const bool blender_mode)
        :DisplayItemBase(title, w, h, blender_mode), x(x), y(y){
        const auto total_pixel = w * h;
        if (is_blender_mode)
            m_data[0] = std::make_unique<float[]>(total_pixel * 4);
        else {
            for (auto i = 0u; i < 3; ++i)
                m_data[i] = std::make_unique<float[]>(total_pixel);
        }
    }

    inline void UpdatePixel(int coord_x, int coord_y, const Spectrum& r) {
        if (is_blender_mode) {
            auto local_index = coord_x + (h - 1 - coord_y) * w;
            m_data[0][4 * local_index] = r[0];
            m_data[0][4 * local_index + 1] = r[1];
            m_data[0][4 * local_index + 2] = r[2];
            m_data[0][4 * local_index + 3] = 1.0f;
        }
        else {
            auto local_index = coord_x + coord_y * w;
            for (auto i = 0u; i < RGBSPECTRUM_SAMPLE; ++i)
                m_data[i][local_index] = r[i];
        }
    }

    void Process(std::unique_ptr<OSocketStream>& stream) override;

protected:
    // data of the tile per channel, we have R/G/B
    std::unique_ptr<float[]>     m_data[SPECTRUM_SAMPLE];
    // the position of the tile
    const int x, y;
};

//! @brief  Indication tile
struct IndicationTile : public DisplayTile {
    IndicationTile(const std::string& title, const int x, const int y,
        const int w, const int h, const bool blender_mode)
        :DisplayTile(title, x, y, w, h, blender_mode) {}
    void Process(std::unique_ptr<OSocketStream>& stream) override;
};

//! @brief  New image to display
struct DisplayImageInfo : public DisplayItemBase {
    DisplayImageInfo(const std::string& title, const int w, const int h, const bool blender_mode)
        :DisplayItemBase(title, w, h, blender_mode){}
    void Process(std::unique_ptr<OSocketStream>& stream) override;
};

//! @brief  Full target update
struct FullTargetUpdate : public DisplayItemBase {
    FullTargetUpdate(const std::string& title, const RenderTarget* rt, const bool is_blender_mode)
        :DisplayItemBase(title, rt->GetWidth(), rt->GetHeight(), is_blender_mode), m_rt(rt) {}
    void Process(std::unique_ptr<OSocketStream>& stream) override;
private:
    const RenderTarget* const m_rt = nullptr;
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
    void SetupDisplayServer(const std::string host, const std::string& port);

    //! @brief  Refresh the tile in display servers
    //!
    //! @param item         A display item to process
    void QueueDisplayItem(std::shared_ptr<DisplayItemBase> item);

    //! @brief  Update display
    //!
    //! @param  cnt         Number of display items to handle at a time, 4 by default. If -1 is set, there is no limitation.
    void ProcessDisplayQueue(int cnt = 0xffffffff);

    //! @brief  Wait for display server to be disconnected
    //!
    //! This will be a blocking call to make sure display server is properly disconnected
    void DisconnectDisplayServer();

    //! @brief  Whether the display server is connected
    //!
    bool IsDisplayServerConnected() const;

private:
    /**< This data structure keeps track of all streams of servers. */
    std::unique_ptr<OSocketStream>                  m_stream;

    /**< Display queue. **/
    std::queue<std::shared_ptr<DisplayItemBase>>    m_queue;
    /**< A spin lock to protect the queue from being accessed in multiple threads. */
    spinlock_mutex                                  m_lock;
    /**< Whether the display server is connected. */
    std::atomic<bool>                               m_display_server_connected;

    /**< The socket of the server. */
    std::unique_ptr<SocketConnection>               m_socket_connection;
};