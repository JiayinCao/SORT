import bpy
import os
import platform
import subprocess
import bl_ui
from . import preference
from . import exporter

bl_info = {
    "name": "SORT",
    "description": "An open-source ray tracer project",
    "author": "Jiayin Cao(Jerry)",
    "version": (0, 0, 1),
    "blender": (2, 75, 0),
    "location": "Info > RenderEngine",
    "warning": "Still under development", # used for warning icon and text in addons panel
    "category": "Render"}

def _register_elm(elm, required=False):
    try:
        elm.COMPAT_ENGINES.add(__name__)
    except:
        if required:
            MtsLog('Failed to add SORT to ' + elm.__name__)

_register_elm(bl_ui.properties_render.RENDER_PT_dimensions, required=True)

class SORT_RENDERER(bpy.types.RenderEngine):
    # These three members are used by blender to set up the
    # RenderEngine; define its internal name, visible name and capabilities.
    bl_idname = __name__
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
            print(sort_bin_path)
            if sort_bin_path is None:
                raise Exception("Set the path where binary for SORT is located before rendering anything.")
            elif not os.path.exists(sort_bin_path):
                raise Exception("SORT not found here: %s"%sort_bin_path)
        except Exception as exc:
            self.sort_available = False
            self.report({'ERROR'},'%s' % exc)

        if not self.sort_available:
            return

        from . import exporter
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
        xres = bpy.data.scenes["Scene"].render.resolution_x
        yres = bpy.data.scenes["Scene"].render.resolution_y
        result = self.begin_result(0, 0, xres, yres)
        lay = result.layers[0]
        lay.load_from_file(exporter.get_immediate_dir() + 'blender_generated.bmp')
        self.end_result(result)

def register():
    print("SORT is enabled in Blender.")

    # Register the RenderEngine
    bpy.utils.register_class(SORT_RENDERER)

    # Register preference
    preference.register()

def unregister():
    # Unregister preference
    preference.unregister()

    # Unregister RenderEngine
    bpy.utils.unregister_class(SORT_RENDERER)

    print("SORT is disabled in Blender.")