import bpy

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

    def __init__(self):
        self.render_pass = None

    def __del__(self):
        if hasattr(self, "render_pass"):
            if self.render_pass != None:
                engine.free(self)

    # update frame
    def update(self, data, scene):
        print("update")

        from . import exporter
        exporter.export_scene(scene);

    # render
    def render(self, scene):
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
    from . import preference
    preference.register()

def unregister():
    # Unregister preference
    from . import preference
    preference.unregister()

    # Unregister RenderEngine
    bpy.utils.unregister_class(SORT_RENDERER)

    print("SORT is disabled in Blender.")