import bpy
import os
import platform
from . import preference

bl_info = {
    "name": "SORT",
    "description": "An open-source ray tracer project",
    "author": "Jiayin Cao(Jerry)",
    "version": (0, 0, 1),
    "blender": (2, 75, 0),
    "location": "Info > RenderEngine",
    "warning": "Still under development", # used for warning icon and text in addons panel
    "category": "Render"}

class SORT_RENDERER(bpy.types.RenderEngine):
    # These three members are used by blender to set up the
    # RenderEngine; define its internal name, visible name and capabilities.
    bl_idname = __name__
    bl_label = 'SORT'
    bl_use_preview = True

    # whether SORT path is set correctly
    sort_available = True

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
            sort_bin_dir = preference.get_sort_path()
            if platform.system() == 'Darwin':   # for Mac OS
                sort_bin_path = sort_bin_dir + "sort"
            elif platform.system() == 'Windows':    # for Windows
                sort_bin_path = sort_bin_dir + "sort.exe"
            else:
                raise Exception("SORT is only supported on Windows, Ubuntu and Mac OS")

            if sort_bin_dir is None:
                raise Exception("Set the path where binary for SORT is located before rendering anything.")
            elif not os.path.exists(sort_bin_path):
                raise Exception("SORT not found here: %s"%sort_bin_dir)
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