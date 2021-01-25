#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.
#
#    SORT is a free software written for educational purpose. Anyone can distribute
#    or modify it under the the terms of the GNU General Public License Version 3 as
#    published by the Free Software Foundation. However, there is NO warranty that
#    all components are functional in a perfect manner. Without even the implied
#    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#    General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along with
#    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.

import bpy
import os
import subprocess
import math
import numpy
import shutil
import threading
import socket
from .log import log, logD
from . import base
from . import exporter

# this thread runs forever
def dipslay_update(sock, render_engine):
    connection, _ = sock.accept()

    while render_engine.terminating_display_thread is False:
        try:
            header_bytes = connection.recv(4)
            if header_bytes == b'':
                print('[header_bytes] socket error.')
                return
            
            pkg_length = int.from_bytes(header_bytes, "little")

            # we are done if the length is zero, this will be the last socket package sent from SORT
            if pkg_length == 0:
                return
            
            header = connection.recv(16)
            if header == b'':
                print('[header] socket error.')
                return

            # update a proportion of the image
            tile_width  = int.from_bytes(header[0:3], "little")
            tile_height = int.from_bytes(header[4:7], "little")
            offset_x    = int.from_bytes(header[8:11], "little")
            offset_y    = int.from_bytes(header[12:15], "little")

            # receive the pixel data
            length_to_read = pkg_length - 16
            pixels = connection.recv(length_to_read, socket.MSG_WAITALL)
            if len(pixels) != length_to_read:
                print('[pixel data] socket error.')
                return

            # convert binary to two dimensional array
            tile_data = numpy.fromstring(pixels, dtype=numpy.float32)
            tile_rect = tile_data.reshape( ( ( tile_width * tile_height ) , 4 ) )

            # begin result
            result = render_engine.begin_result(offset_x, render_engine.image_size_h - offset_y - 1 - tile_height, tile_width, tile_height)

            # update image memmory
            if result is not None:
                result.layers[0].passes[0].rect = tile_rect

                # refresh the update
                render_engine.end_result(result)

        except socket.error as e:
            print('socket error\t ')
            print(e)
            break
    
    connection.close()

@base.register_class
class SORTRenderEngine(bpy.types.RenderEngine):
    # These three members are used by blender to set up the
    # RenderEngine; define its internal name, visible name and capabilities.
    bl_idname = 'SORT'
    bl_label = 'SORT'
    bl_use_preview = False  # disable material preview until it works

    sock        = None
    host_name   = socket.gethostname()
    ip_addr     = socket.gethostbyname(host_name)
    port        = 2009 # just a random port

    display_thread = None

    render_lock = threading.Lock()

    terminating_display_thread = False

    @classmethod
    def is_active(cls, context):
        return context.scene.render.engine == cls.bl_idname

    def __init__(self):
        self.sort_available = True
        self.cmd_argument = []
        self.render_pass = None
        
        self.image_tile_size = 64
        self.image_size_w = int(bpy.data.scenes[0].render.resolution_x * bpy.data.scenes[0].render.resolution_percentage / 100)
        self.image_size_h = int(bpy.data.scenes[0].render.resolution_y * bpy.data.scenes[0].render.resolution_percentage / 100)
        self.image_pixel_count = self.image_size_w * self.image_size_h
        self.image_tile_count_x = math.ceil( self.image_size_w / self.image_tile_size )
        self.image_tile_count_y = math.ceil( self.image_size_h / self.image_tile_size )
        self.image_header_size = self.image_tile_count_x * self.image_tile_count_y
        self.image_tile_pixel_count = self.image_tile_size * self.image_tile_size
        self.image_tile_size_in_bytes = self.image_tile_pixel_count * 16
        self.image_size_in_bytes = self.image_tile_count_x * self.image_tile_count_y * self.image_tile_size_in_bytes

        # create the socket
        if self.sock is None:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

            # non blocking socket
            self.sock.setblocking(False)

            # bind the socket
            self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.sock.bind((self.ip_addr, self.port))

            # listen for socket connection
            self.sock.settimeout(5)
            self.sock.listen()

            log("Your Computer Name is:\t\t" + self.host_name)
            log("Listening address and port:\t {ip}:{p}".format(ip=self.ip_addr, p=self.port))

    # update frame
    def update(self, data, depsgraph):
        # check if the path for SORT is set correctly
        try:
            self.sort_available = True
            sort_bin_path = exporter.get_sort_bin_path()
            if sort_bin_path is None:
                raise Exception("Set the path where binary for SORT is located before rendering anything.")
            elif not os.path.exists(sort_bin_path):
                raise Exception("SORT not found here: %s"%sort_bin_path)
        except Exception as exc:
            self.sort_available = False
            self.report({'ERROR'},'%s' % exc)

        if not self.sort_available:
            return

        # export the scene
        exporter.export_blender(depsgraph)

    # render
    def render(self, depsgraph):
        scene = depsgraph.scene
        if not self.sort_available:
            return

        with SORTRenderEngine.render_lock:
            if scene.name == 'preview':
                # this is where we handle preview, but it is not implemented
                pass
            else:
                self.render_scene(scene)

    # scene render
    def render_scene(self, scene):
        # start rendering process first
        binary_dir = exporter.get_sort_dir()
        binary_path = exporter.get_sort_bin_path()
        intermediate_dir = exporter.get_intermediate_dir()
        # execute binary
        self.cmd_argument = [binary_path];
        self.cmd_argument.append( '--input:' + intermediate_dir + 'scene.sort')
        self.cmd_argument.append( "--displayserver:" + self.ip_addr + ":" + str(self.port))
        self.cmd_argument.append( '--blendermode' )
        if scene.sort_data.profilingEnabled is True:
            self.cmd_argument.append( '--profiling:on' )
        if scene.sort_data.allUseDefaultMaterial is True:
            self.cmd_argument.append( '--noMaterial' )
        process = subprocess.Popen(self.cmd_argument,cwd=binary_dir)

        # start a background pool thread
        self.display_thread = threading.Thread(target=dipslay_update, args=(self.sock, self))
        self.display_thread.start()

        # wait for the process to finish
        while subprocess.Popen.poll(process) is None:
            if self.test_break():
                break

        # terminate the process by force
        if subprocess.Popen.poll(process) is None:
            # indiate to terminate the thread since the exe is about to be terminated
            self.terminating_display_thread = True

            # wait for the thread to be terminated before moving forward
            self.display_thread.join()

            # terminate the process
            subprocess.Popen.terminate(process)

        # clear immediate directory
        shutil.rmtree(intermediate_dir)
