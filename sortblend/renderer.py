#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
# 
#    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
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
from .exporter import sort_exporter
from extensions_framework.util import TimerThread
from . import base

class SORT_Thread(TimerThread):
    render_engine = None
    shared_memory = None
    float_shared_memory = None

    def setrenderengine(self, re):
        self.render_engine = re

    def setsharedmemory(self,sm):
        # setup shared memory
        self.shared_memory = sm

        # pack image content as float
        self.float_shared_memory = struct.pack('%sf'%(self.render_engine.image_size_in_bytes), *sm[self.render_engine.image_header_size:self.render_engine.image_size_in_bytes + self.render_engine.image_header_size] )

    def kick(self, render_end=False):
        self.update()

    def update(self, final_update=False):
        # total pixel count
        mod = self.render_engine.image_tile_size - ( self.render_engine.image_size_h % self.render_engine.image_tile_size )
        if mod is self.render_engine.image_tile_size:
            mod = 0

        # pick active tiles to update
        active_tiles = self.picknewtiles()

        for i in active_tiles:
            tile_x = i % self.render_engine.image_tile_count_x
            tile_y = int(i / self.render_engine.image_tile_count_x)

            tile_x_offset = tile_x * self.render_engine.image_tile_size
            tile_y_offset = tile_y * self.render_engine.image_tile_size

            tile_size_x = min( self.render_engine.image_tile_size , self.render_engine.image_size_w - tile_x_offset )
            tile_size_y = self.render_engine.image_tile_size

            # y offset
            offset_y = max( mod - tile_y_offset , 0 )

            # load shared memory
            self.shared_memory.seek( self.render_engine.image_header_size + i * self.render_engine.image_tile_size_in_bytes + offset_y * tile_size_x * 16)
            byptes = self.shared_memory.read(self.render_engine.image_tile_size_in_bytes - offset_y * tile_size_x * 16)

            # convert binary to two dimensional array
            tile_data = numpy.fromstring(byptes, dtype=numpy.float32)
            tile_rect = tile_data.reshape( ( ( self.render_engine.image_tile_pixel_count - offset_y * tile_size_x ) , 4 ) )

            # begin result
            result = self.render_engine.begin_result(tile_x_offset, max(tile_y_offset - mod,0), tile_size_x, tile_size_y - offset_y)

            # update image memmory
            result.layers[0].passes[0].rect = tile_rect

            # refresh the update
            self.render_engine.end_result(result)

            # update header info to make sure it is not processed again
            self.shared_memory[i] = self.shared_memory[i] + 1

        # close the shared memory if it is the last update
        #if final_update:
        #    self.shared_memory.close()

    def picknewtiles(self):
        active_tiles = []
        for i in range( self.render_engine.image_header_size ):
            if self.shared_memory[i] is 1:
                active_tiles.append(i)

        return active_tiles

@base.register_class
class SORT_RENDERER(bpy.types.RenderEngine):
    # These three members are used by blender to set up the
    # RenderEngine; define its internal name, visible name and capabilities.
    bl_idname = 'SORT_RENDERER'
    bl_label = 'SORT'
    bl_use_preview = True

    render_lock = threading.Lock()

    # spawn new rendering thread
    def spawnnewthread(self):
        import mmap

        # setup shared memory size
        self.sm_size = self.image_size_in_bytes * 2 + self.image_header_size + 2
 
        intermediate_dir = sort_exporter.get_intermediate_dir()
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
        self.sort_thread.set_kick_period(1)
        self.sort_thread.start()

    def __init__(self):
        self.sort_available = True
        self.cmd_argument = []
        self.render_pass = None
        self.sort_thread = SORT_Thread()
        self.sort_thread.setrenderengine(self)
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
    def update(self, data, scene):
        # check if the path for SORT is set correctly
        try:
            self.sort_available = True
            sort_bin_path = sort_exporter.get_sort_bin_path()
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
        sort_exporter.export_blender(scene)

    # render
    def render(self, scene):
        if not self.sort_available:
            return

        with SORT_RENDERER.render_lock:
            if scene.name == 'preview':
                self.render_preview(scene)
            else:
                self.render_scene(scene)

    # preview render
    def render_preview(self, scene):
        #spawn new thread
        self.spawnnewthread()

        # start rendering process first
        binary_dir = sort_exporter.get_sort_dir()
        binary_path = sort_exporter.get_sort_bin_path()
        intermediate_dir = sort_exporter.get_intermediate_dir()
        # execute binary
        self.cmd_argument = [binary_path];
        self.cmd_argument.append( intermediate_dir + 'sort_scene.xml')
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
            print(output_file)
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
        binary_dir = sort_exporter.get_sort_dir()
        binary_path = sort_exporter.get_sort_bin_path()
        intermediate_dir = sort_exporter.get_intermediate_dir()
        # execute binary
        self.cmd_argument = [binary_path];
        self.cmd_argument.append( intermediate_dir + 'sort_scene.xml')
        self.cmd_argument.append('blendermode')
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
            self.sort_thread.update(True)

            # if final update is necessary
            final_update = self.sharedmemory[self.image_size_in_bytes * 2 + self.image_header_size + 1]
            if final_update:
                # begin result
                result = self.begin_result(0, 0, bpy.data.scenes[0].render.resolution_x, bpy.data.scenes[0].render.resolution_y)

                print(result)
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