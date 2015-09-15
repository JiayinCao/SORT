import bpy
import os
import subprocess
import math
import struct
import numpy
from . import exporter
from . import preference
from . import common
from extensions_framework.util import TimerThread

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
        if final_update:
            self.shared_memory.close()

    def picknewtiles(self):
        active_tiles = []
        for i in range( self.render_engine.image_header_size ):
            if self.shared_memory[i] is 1:
                active_tiles.append(i)

        return active_tiles

class SORT_RENDERER(bpy.types.RenderEngine):
    # These three members are used by blender to set up the
    # RenderEngine; define its internal name, visible name and capabilities.
    bl_idname = common.default_bl_name
    bl_label = 'SORT'
    bl_use_preview = True

    # spawn new rendering thread
    def spawnnewthread(self):
        # allocate shared memory first
        import mmap
        self.sharedmemory = mmap.mmap(0, self.image_size_in_bytes + self.image_header_size + 1 , "SORTBLEND_SHAREMEM")
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

    def __del__(self):
        print('delete')

    # update frame
    def update(self, data, scene):
        # check if the path for SORT is set correctly
        try:
            self.sort_available = True
            sort_bin_path = preference.get_sort_bin_path()
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
        exporter.export_blender(scene);

    # render
    def render(self, scene):
        if not self.sort_available:
            return

        if scene.name == 'preview':
            self.render_preview(scene)
        else:
            self.render_scene(scene)

    # preview render
    def render_preview(self, scene):
        print("render_preview")

    # scene render
    def render_scene(self, scene):
        #spawn new thread
        self.spawnnewthread()

        # start rendering process first
        binary_dir = preference.get_sort_dir()
        binary_path = preference.get_sort_bin_path()

        # execute binary
        self.cmd_argument = [binary_path];
        self.cmd_argument.append('./blender_intermediate/blender_exported.xml')
        self.cmd_argument.append('blendermode')
        print(self.cmd_argument)
        process = subprocess.Popen(self.cmd_argument,cwd=binary_dir)

        # wait for the process to finish
        while subprocess.Popen.poll(process) is None:
            if self.test_break():
                break
            progress = self.sharedmemory[self.image_size_in_bytes + self.image_header_size]
            self.update_progress(progress/100)

        # terminate the process by force
        if subprocess.Popen.poll(process) is None:
            subprocess.Popen.terminate(process)

        # wait for the thread to finish
        if self.sort_thread.isAlive():
            self.sort_thread.stop()
            self.sort_thread.join()
            self.sort_thread.update(True)

def register():
    # Register the RenderEngine
    bpy.utils.register_class(SORT_RENDERER)

def unregister():
    # Unregister RenderEngine
    bpy.utils.unregister_class(SORT_RENDERER)