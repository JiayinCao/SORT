import bpy
import os
import subprocess
from . import preference
from . import exporter
from . import preference
from . import common

class SORT_RENDERER(bpy.types.RenderEngine):
    # These three members are used by blender to set up the
    # RenderEngine; define its internal name, visible name and capabilities.
    bl_idname = common.default_bl_name
    bl_label = 'SORT'
    bl_use_preview = True

    # whether SORT path is set correctly
    sort_available = True
    # command argument
    cmd_argument = []
    # thread
    fb_thread = None

    def __init__(self):
        self.render_pass = None

    def __del__(self):
        if hasattr(self, "render_pass"):
            if self.render_pass != None:
                engine.free(self)

    # update frame
    def update(self, data, scene):
        print("starting update")
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

        print("starting render")

        if scene.name == 'preview':
            self.render_preview(scene)
        else:
            self.render_scene(scene)

    # preview render
    def render_preview(self, scene):
        print("render_preview")

    # scene render
    def render_scene(self, scene):
        print("render_scene")

        # start rendering process first
        binary_dir = preference.get_sort_dir()
        binary_path = preference.get_sort_bin_path()

        # execute binary
        self.cmd_argument = [binary_path];
        self.cmd_argument.append('./blender_intermediate/blender_exported.xml')
        process = subprocess.Popen(self.cmd_argument,cwd=binary_dir)
        # wait for the process to finish
        subprocess.Popen.wait(process)

        # load the result from file
        xres = bpy.data.scenes[0].render.resolution_x * bpy.data.scenes[0].render.resolution_percentage / 100
        yres = bpy.data.scenes[0].render.resolution_y * bpy.data.scenes[0].render.resolution_percentage / 100
        result = self.begin_result(0, 0, xres, yres)
        lay = result.layers[0]
        result_filename = preference.get_immediate_dir() + 'blender_generated.exr'
        if os.path.isfile(result_filename):
            lay.load_from_file(result_filename)
        self.end_result(result)

def register():
    # Register the RenderEngine
    bpy.utils.register_class(SORT_RENDERER)

def unregister():
    # Unregister RenderEngine
    bpy.utils.unregister_class(SORT_RENDERER)