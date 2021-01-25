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

#include "display_mgr.h"
#include "core/globalconfig.h"
#include "core/socket_mgr.h"
#include "core/sassert.h"

#ifdef SORT_IN_WINDOWS
#else
#define INVALID_SOCKET (-1)
#endif

enum Type : char {
    OpenImage = 0,
    ReloadImage = 1,
    CloseImage = 2,
    UpdateImage = 3,
    CreateImage = 4,
};

static std::string  channelNames[] = { "R", "G", "B" };
static int          nChannels = 3;

void DisplayManager::AddDisplayServer(const std::string host, const std::string& port) {
    // It is fairly easy to support multiple display servers, but it makes little sense for me for now.
    // So I will ignore the second one coming in.
    if (m_stream)
        return;

    auto socket = SocketManager::GetSingleton().AddSocket(SOCKET_TYPE::CLIENT, host, port);
    m_stream = std::make_unique<OSocketStream>(socket);
}

bool DisplayManager::IsDisplayServerConnected() const {
    return m_stream != nullptr;
}

void DisplayManager::ProcessDisplayQueue(int cnt) {
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

void DisplayTile::Process(std::unique_ptr<OSocketStream>& ptr_stream) {
    OSocketStream& stream = *ptr_stream;

    if (g_blenderMode){
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
        for (auto i = 0u; i < nChannels; ++i) {
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
    if (!g_blenderMode) {
        OSocketStream& stream = *ptr_stream;

        const int image_width = g_resultResollutionWidth;
        const int image_height = g_resultResollutionHeight;
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

void TerminateIndicator::Process(std::unique_ptr<OSocketStream>& ptr_stream) {
    // Tev won't response this well
    if (g_blenderMode) {
        // Blender doesn't care about creating a new image, only TEV does.
        OSocketStream& stream = *ptr_stream;
        stream << int(0);   // 0 as length indicating that we are done, no more package will be received.
        stream.Flush();
    }
}

void FullTargetUpdate::Process(std::unique_ptr<OSocketStream>& ptr_stream) {
    // This is slow, but so far it is only used in light tracing algorithm, an algorithm that I rarely used.
    // So I don't care about its performance.
    // For bidirecitonal path tracing, by the time it is needed, it is already finished, so it is not a performance issue.
    // WARNING, this thread might result in some unknown results because of unguarded data racing. However, it is not a big
    // deal to reveal slightly inconsistent data as long as the final result is fine.

    OSocketStream& stream = *ptr_stream;

    auto w = g_imageSensor->GetWidth();
    auto h = g_imageSensor->GetHeight();

    if (g_blenderMode) {
        // [0] Length of the package, it doesn't count itself
        // [1] Width of the tile
        // [2] Height of the tile
        // [3] x position of the tile
        // [4] y position of the tile
        // [....] the pixel data
        const int pixel_memory_size = w * h * sizeof(float) * 4;
        const int total_size = pixel_memory_size + sizeof(int) * 4;
        stream << int(total_size);
        stream << w << h << int(0) << int(0);

        std::unique_ptr<float[]> data = std::make_unique<float[]>(w * h * 4);
        for (auto i = 0; i < h; ++i) {
            for (auto j = 0; j < w; ++j) {
                const auto index = 4 * ((h - i - 1) * w + j);
                const auto src_index = i * w + j;
                data[index]     = g_imageSensor->m_rendertarget.m_pData[src_index][0];
                data[index+1]   = g_imageSensor->m_rendertarget.m_pData[src_index][1];
                data[index+2]   = g_imageSensor->m_rendertarget.m_pData[src_index][2];
                data[index+3]   = 1.0f;
            }
        }
        stream.Write((char*)data.get(), pixel_memory_size);
        stream.Flush();
    }
    else {
        std::unique_ptr<float[]> data;
        data = std::make_unique<float[]>(w * h);

        // This is for TEV
        // https://github.com/Tom94/tev
        for (auto c = 0u; c < nChannels; ++c) {
            stream << int(0);            // reserved for length
            stream << char(UpdateImage); // indicate to update some of the images
            stream << char(0);           // indicate to grab the current image
            stream << title;             // indicate the title of the image
            stream << channelNames[c];   // the channel name

            stream << int(0) << int(0) << w << h;

            for (auto i = 0; i < h; ++i) {
                for (auto j = 0; j < w; ++j) {
                    const auto index = i * w + j;
                    data[index] = g_imageSensor->m_rendertarget.m_pData[index][c];
                }
            }

            stream.Write((char*)data.get(), w * h * sizeof(float));
           
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