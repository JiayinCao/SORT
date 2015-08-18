import bpy
from bpy.types import AddonPreferences
from bpy.props import StringProperty, IntProperty, BoolProperty

import platform
import os
import subprocess
from subprocess import Popen, PIPE

bl_info = {
    "name": "SORT",
    "description": "An open-source ray tracer project",
    "author": "Jiayin Cao(Jerry)",
    "version": (0, 0, 1),
    "blender": (2, 75, 0),
    "location": "Info > RenderEngine",
    "warning": "Still under development", # used for warning icon and text in addons panel
    "category": "Render"}

class SORTAddonPreferences(AddonPreferences):
    # this must match the addon name
    bl_idname = __name__
    install_path = StringProperty(
            name="Path to SORT binary",
            description='Path to SORT binary',
            subtype='DIR_PATH',
            default="",
            )

    def draw(self, context):
        layout = self.layout
        layout.prop(self, "install_path")

class SORT_RENDERER(bpy.types.RenderEngine):
    # These three members are used by blender to set up the
    # RenderEngine; define its internal name, visible name and capabilities.
    bl_idname = 'sort_renderer'
    bl_label = 'SORT'
    bl_use_preview = True

    # This is the only method called by blender, in this example
    # we use it to detect preview rendering and call the implementation
    # in another method.
    def render(self, scene):
        scale = scene.render.resolution_percentage / 100.0
        self.size_x = int(scene.render.resolution_x * scale)
        self.size_y = int(scene.render.resolution_y * scale)

        if scene.name == 'preview':
            self.render_preview(scene)
        else:
            self.render_scene(scene)

    # In this example, we fill the preview renders with a flat green color.
    def render_preview(self, scene):
        pixel_count = self.size_x * self.size_y

        # The framebuffer is defined as a list of pixels, each pixel
        # itself being a list of R,G,B,A values
        green_rect = [[1.0, 1.0, 1.0, 1.0]] * pixel_count

        # Here we write the pixel values to the RenderResult
        result = self.begin_result(0, 0, self.size_x, self.size_y)
        layer = result.layers[0]
        layer.rect = green_rect
        self.end_result(result)

    # In this example, we fill the full renders with a flat blue color.
    def render_scene(self, scene):
        pixel_count = self.size_x * self.size_y

        # The framebuffer is defined as a list of pixels, each pixel
        # itself being a list of R,G,B,A values
        blue_rect = [[1.0, 1.0, 1.0, 1.0]] * pixel_count

        # Here we write the pixel values to the RenderResult
        result = self.begin_result(0, 0, self.size_x, self.size_y)

        #print(platform.system())
        #environ = os.environ.copy()
        #exepath = 'E:\\apitest\\apitest\\bin\\apitest_d.exe'
        #print(exepath)
        #subprocess.Popen([exepath], env=environ, shell=True)

        layer = result.layers[0]
        #layer.rect = blue_rect somehow, it crashes at this line of code

        print("render is done")
        print(__name__)
        self.end_result(result)

def register():
    # register path setting
    #from . import next
    #next.register()
    bpy.utils.register_class(SORTAddonPreferences)
    #preference.register()
    print("SORT is enabled in Blender.")

    # Register the RenderEngine
    bpy.utils.register_class(SORT_RENDERER)
    
    from bl_ui import properties_render
    properties_render.RENDER_PT_render.COMPAT_ENGINES.add('sort_renderer')
    del properties_render

    from bl_ui import properties_material
    properties_material.MATERIAL_PT_preview.COMPAT_ENGINES.add('sort_renderer')
    del properties_material

def unregister():
    #unregister path setting
    #from . import preferences
    #preference.unregister()
    bpy.utils.unregister_class(SORTAddonPreferences)
    #unregister RenderEngine
    bpy.utils.unregister_class(SORT_RENDERER)
    print("SORT is disabled in Blender.")