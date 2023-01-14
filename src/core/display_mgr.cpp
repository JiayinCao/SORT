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

#include <mutex>
#include "display_mgr.h"
#include "core/sassert.h"
#include "texture/rendertarget.h"

enum Type : char {
    OpenImage = 0,
    ReloadImage = 1,
    CloseImage = 2,
    UpdateImage = 3,
    CreateImage = 4,
};

static std::string  channelNames[] = { "R", "G", "B" };
static int          nChannels = 3;

void DisplayManager::SetupDisplayServer(const std::string host, const std::string& port) {
    m_socket_connection = ConnectSocket(host, port);

    if (m_socket_connection) {
        slog(INFO, SOCKET, "Connected to display server %s:%s.", host.c_str(), port.c_str());
        m_stream = std::make_unique<OSocketStream>(m_socket_connection->m_socket);
        m_display_server_connected = true;
    }
}

bool DisplayManager::IsDisplayServerConnected() const {
    return m_display_server_connected;
}

void DisplayManager::ProcessDisplayQueue(int cnt) {
    // don't process anything if the server is not even connected
    if (!m_display_server_connected)
        return;

    // we only process 4 display tiles everytime this thread gains control
    while (cnt-- != 0) {
        std::shared_ptr<DisplayItemBase> item;
       
        // grab one from the queue
        {
            std::lock_guard<spinlock_mutex> guard(m_lock);

            // make sure there is still things left to do
            if (m_queue.empty())
                break;
            
            item = m_queue.front();
            m_queue.pop();
        }

        item->Process(m_stream);
    }
}

void DisplayManager::QueueDisplayItem(std::shared_ptr<DisplayItemBase> item) {
    std::lock_guard<spinlock_mutex> guard(m_lock);
    m_queue.push(item);
}

void DisplayManager::DisconnectDisplayServer() {
    if (m_display_server_connected) {
        slog(INFO, SOCKET, "Disconnect display server.");
        DisconnectSocket(m_socket_connection.get());
    }
}

void DisplayTile::Process(std::unique_ptr<OSocketStream>& ptr_stream) {
    OSocketStream& stream = *ptr_stream;

    if (is_blender_mode){
        // [0] Length of the package, it doesn't count itself
        // [1] Width of the tile
        // [2] Height of the tile
        // [3] x position of the tile
        // [4] y position of the tile
        // [....] the pixel data
        const int pixel_memory_size = w * h * sizeof(float) * 4;
        const int total_size = pixel_memory_size + sizeof(int) * 4;
        stream << int(total_size);
        stream << w << h << x << y;
        stream.Write((char*)m_data->get(), pixel_memory_size);
        stream.Flush();
    } else {
        // This is for TEV
        // https://github.com/Tom94/tev
        for (auto i = 0; i < nChannels; ++i) {
            stream << int(0);            // reserved for length
            stream << char(UpdateImage); // indicate to update some of the images
            stream << char(0);           // indicate to grab the current image
            stream << title;             // indicate the title of the image
            stream << channelNames[i];   // the channel name

            stream << x << y << w << h;
            stream.Write((char*)m_data[i].get(), w * h * sizeof(float));

            // set the length data
            const auto pos = stream.GetPos();
            stream.Seek(0);
            stream << int(pos);
            stream.Seek(pos);

            // distribute data to all display servers
            stream.Flush();
        }
    }
}

void DisplayImageInfo::Process(std::unique_ptr<OSocketStream>& ptr_stream) {
    // Blender doesn't care about creating a new image, only TEV does.
    if (!is_blender_mode) {
        OSocketStream& stream = *ptr_stream;

        const int image_width = w;
        const int image_height = h;
        stream << int(0);            // reserved for length
        stream << char(CreateImage); // indicate to update some of the images
        stream << char(1);           // indicate to grab the current image
        stream << title;             // indicate the title of the image
        stream << image_width << image_height;
        stream << int(nChannels);
        for (auto i = 0; i < nChannels; ++i)
            stream << channelNames[i];   // the channel name

        // set the length data
        const auto pos = stream.GetPos();
        stream.Seek(0);
        stream << int(pos);
        stream.Seek(pos);

        // flush the data
        stream.Flush();
    }
}

void IndicationTile::Process(std::unique_ptr<OSocketStream>& ptr_stream) {
    const auto indication_intensity = 0.3f;
    const auto total_pixel = w * h;
    if (is_blender_mode) {
        auto data = m_data[0].get();
        memset(data, 0, sizeof(float) * total_pixel * 4);

        for (auto i = 0; i < w; ++i) {
            if ((i >> 2) % 2 == 0)
                continue;

            for (auto c = 0; c < 3; ++c) {
                data[4 * i + c] = indication_intensity;
                data[4 * (total_pixel - 1 - i) + c] = indication_intensity;
            }
            data[4 * i + 3] = 1.0f;
            data[4 * (total_pixel - 1 - i) + 3] = 1.0f;
        }
        for (auto i = 0; i < h; ++i) {
            if ((i >> 2) % 2 == 0)
                continue;

            for (auto c = 0; c < 3; ++c) {
                data[4 * (i * w) + c] = indication_intensity;
                data[4 * (i * w + w - 1) + c] = indication_intensity;
            }
            data[4 * (i * w) + 3] = 1.0f;
            data[4 * (i * w + w - 1) + 3] = 1.0f;
        }
    }
    else {
        for (auto i = 0u; i < RGBSPECTRUM_SAMPLE; ++i) {
            auto data = m_data[i].get();
            memset(data, 0, sizeof(float) * total_pixel);

            for (auto i = 0; i < w; ++i) {
                if ((i >> 2) % 2 == 0)
                    continue;
                data[i] = indication_intensity;
                data[total_pixel - 1 - i] = indication_intensity;
            }
            for (auto i = 0; i < h; ++i) {
                if ((i >> 2) % 2 == 0)
                    continue;
                data[i * w] = indication_intensity;
                data[i * w + w - 1] = indication_intensity;
            }
        }
    }

    DisplayTile::Process(ptr_stream);
}

void FullTargetUpdate::Process(std::unique_ptr<OSocketStream>& ptr_stream) {
    // This is slow, but so far it is only used in light tracing algorithm, an algorithm that I rarely used.
    // So I don't care about its performance.
    // For bidirecitonal path tracing, by the time it is needed, it is already finished, so it is not a performance issue.
    // WARNING, this thread might result in some unknown results because of unguarded data racing. However, it is not a big
    // deal to reveal slightly inconsistent data as long as the final result is fine.

    const auto total_pixel = w * h;
    auto display_tile = std::make_shared<DisplayTile>(title, 0, 0, w, h, is_blender_mode);
    for( auto i = 0; i < h;  ++i ){
        for( auto j = 0; j < w; ++j ){
            const auto& color = m_rt->GetColor(j, i);
            display_tile->UpdatePixel(j, i, color);
        }
    }
    display_tile->Process(ptr_stream);
}