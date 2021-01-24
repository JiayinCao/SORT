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
import struct
import numpy
import shutil
import platform
import threading
import time
import socket
from .log import log, logD
from . import base
from . import exporter

class SORT_Thread():
    render_engine = None
    shared_memory = None
    float_shared_memory = None

    sock = None
    host_name = socket.gethostname()
    ip_addr = socket.gethostbyname(host_name)
    port    = 2006 # just a random port

    def __init__(self, engine):
        self.isTerminated = False
        self.render_engine = engine
        self.thread = threading.Thread(name="Rendering Thread", target=self.update)

    def listen_socket(self):
        ip_addr = self.ip_addr
        port = self.port

        log("Your Computer Name is:\t\t" + self.host_name)
        log("Listening address and port:\t {ip}:{p}".format(ip=ip_addr, p=port))

        # create the socket
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # non blocking socket
        self.sock.setblocking(False)

        # bind the socket
        #self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind((ip_addr, port))

        # listen for socket connection
        self.sock.settimeout(20)
        self.sock.listen()

        sort_addr = None
        try:
            self.connection, sort_addr = self.sock.accept()
        except socket.timeout:
            log("Timeout exceeded")
            return

        log('SORT is connected!')

        self.thread.start()
        
    def join(self):
        self.thread.join()

    def stop(self):
        self.isTerminated = True

    def isAlive(self):
        return self.thread.isAlive()

    def setsharedmemory(self,sm):
        # setup shared memory
        self.shared_memory = sm

        # pack image content as float
        self.float_shared_memory = struct.pack('%sf'%(self.render_engine.image_size_in_bytes), *sm[self.render_engine.image_header_size:self.render_engine.image_size_in_bytes + self.render_engine.image_header_size] )

    def update(self, final_update=False):
        while True:
            try:
                header_bytes = self.connection.recv(4)
                if header_bytes == b'':
                    print('[header_bytes] socket error.')
                
                pkg_length = int.from_bytes(header_bytes, "little")

                # we are done if the length is zero, this will be the last socket package sent from SORT
                if pkg_length == 0:
                    break
                
                header = self.connection.recv(16)
                if header == b'':
                    print('[header] socket error.')

                # update a proportion of the image
                tile_width  = int.from_bytes(header[0:3], "little")
                tile_height = int.from_bytes(header[4:7], "little")
                offset_x    = int.from_bytes(header[8:11], "little")
                offset_y    = int.from_bytes(header[12:15], "little")

                # receive the pixel data
                pixels = self.connection.recv(pkg_length - 16)

                if pixels == b'':
                    print('[pixel data] socket error.')

                # convert binary to two dimensional array
                tile_data = numpy.fromstring(pixels, dtype=numpy.float32)
                tile_rect = tile_data.reshape( ( ( tile_width * tile_height ) , 4 ) )

                # begin result
                result = self.render_engine.begin_result(offset_x, self.render_engine.image_size_h - offset_y - 1 - tile_height, tile_width, tile_height)

                # update image memmory
                if result is not None:
                    result.layers[0].passes[0].rect = tile_rect

                    # refresh the update
                    self.render_engine.end_result(result)

            except socket.error as e:
                print('socket error\t ')
                print(e)
                break
        
        # we are done with rendering, no need for the socket anymore
        self.sock.close()

@base.register_class
class SORTRenderEngine(bpy.types.RenderEngine):
    # These three members are used by blender to set up the
    # RenderEngine; define its internal name, visible name and capabilities.
    bl_idname = 'SORT'
    bl_label = 'SORT'
    bl_use_preview = False  # disable material preview until it works

    render_lock = threading.Lock()

    @classmethod
    def is_active(cls, context):
        return context.scene.render.engine == cls.bl_idname

    # spawn new rendering thread
    def spawnnewthread(self):
        import mmap

        # setup shared memory size
        self.sm_size = self.image_size_in_bytes * 2 + self.image_header_size + 2

        intermediate_dir = exporter.get_intermediate_dir()
        sm_full_path = intermediate_dir + "sharedmem.bin"
        # on mac os
        if platform.system() == "Darwin" or platform.system() == "Linux":
            # open a new file
            self.file = open( sm_full_path , "wb" , self.sm_size)
            self.file.write( bytes( "\0" * self.sm_size , "utf-8" ) )
            self.file.close()

            # open it in append mode
            self.file = open( sm_full_path , "a+b" , self.sm_size)

            # allocate shared memory first
            self.sharedmemory = mmap.mmap(self.file.fileno(), self.sm_size)
        elif platform.system() == "Windows":
            self.sharedmemory = mmap.mmap(0, self.sm_size , sm_full_path)

        self.sort_thread.setsharedmemory(self.sharedmemory)

    def __init__(self):
        self.sort_available = True
        self.cmd_argument = []
        self.render_pass = None
        self.sort_thread = SORT_Thread(self)
        self.sharedmemory = None

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
                self.render_preview(scene)
            else:
                self.render_scene(scene)

    # preview render
    def render_preview(self, depsgraph):
        scene = depsgraph.scene
        #spawn new thread
        self.spawnnewthread()

        # start rendering process first
        binary_dir = exporter.get_sort_dir()
        binary_path = exporter.get_sort_bin_path()
        intermediate_dir = exporter.get_intermediate_dir()
        # execute binary
        self.cmd_argument = [binary_path]
        self.cmd_argument.append( "--displayserver:" + self.sort_thread.ip_addr + ":" + str(self.sort_thread.port))
        self.cmd_argument.append( intermediate_dir + 'scene.sort')
        process = subprocess.Popen(self.cmd_argument,cwd=binary_dir)

        # wait for the process to finish
        while subprocess.Popen.poll(process) is None:
            if self.test_break():
                break
            progress = self.sharedmemory[self.image_size_in_bytes * 2 + self.image_header_size]
            self.update_progress(progress/100)

        # terminate the process by force
        if subprocess.Popen.poll(process) is None:
            subprocess.Popen.terminate(process)

        # wait for the thread to finish
        if self.sort_thread.isAlive():
            self.sort_thread.stop()
            self.sort_thread.join()

            # begin result
            result = self.begin_result(0, 0, scene.render.resolution_x, scene.render.resolution_y)

            # update image memory
            output_file = intermediate_dir + 'blender_generated.exr'
            result.layers[0].load_from_file(output_file)

            # refresh the update
            self.end_result(result)

            # close shared memory connection
            self.sharedmemory.close()

        # clear immediate directory
        shutil.rmtree(intermediate_dir)

    # scene render
    def render_scene(self, scene):
        #spawn new thread
        self.spawnnewthread()

        # start rendering process first
        binary_dir = exporter.get_sort_dir()
        binary_path = exporter.get_sort_bin_path()
        intermediate_dir = exporter.get_intermediate_dir()
        # execute binary
        self.cmd_argument = [binary_path];
        self.cmd_argument.append( '--input:' + intermediate_dir + 'scene.sort')
        self.cmd_argument.append( "--displayserver:" + self.sort_thread.ip_addr + ":" + str(self.sort_thread.port))
        self.cmd_argument.append( '--blendermode' )
        if scene.sort_data.profilingEnabled is True:
            self.cmd_argument.append( '--profiling:on' )
        if scene.sort_data.allUseDefaultMaterial is True:
            self.cmd_argument.append( '--noMaterial' )
        process = subprocess.Popen(self.cmd_argument,cwd=binary_dir)

        # start listening the socket
        self.sort_thread.listen_socket()

        # wait for the process to finish
        while subprocess.Popen.poll(process) is None:
            if self.test_break():
                break
            progress = self.sharedmemory[self.image_size_in_bytes * 2 + self.image_header_size]
            self.update_progress(progress/100)

        # terminate the process by force
        if subprocess.Popen.poll(process) is None:
            subprocess.Popen.terminate(process)

        # wait for the thread to finish
        self.sort_thread.stop()
        self.sort_thread.join()

        # if final update is necessary
        final_update = self.sharedmemory[self.image_size_in_bytes * 2 + self.image_header_size + 1]
        if final_update and False:
            # begin result
            result = self.begin_result(0, 0, bpy.data.scenes[0].render.resolution_x, bpy.data.scenes[0].render.resolution_y)

            self.sharedmemory.seek( self.image_header_size + self.image_size_in_bytes)
            byptes = self.sharedmemory.read(self.image_pixel_count * 16)

            tile_data = numpy.fromstring(byptes, dtype=numpy.float32)
            tile_rect = tile_data.reshape( self.image_pixel_count , 4 )

            # update image memory
            result.layers[0].passes[0].rect = tile_rect

            # refresh the update
            self.end_result(result)

        # close shared memory connection
        self.sharedmemory.close()

        # clear immediate directory
        shutil.rmtree(intermediate_dir)
