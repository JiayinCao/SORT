import bpy

bl_info = {
    "name": "SORT",
    "description": "An open-source ray tracer project",
    "author": "Jiayin Cao(Jerry)",
    "version": (1, 0),
    "blender": (2, 75, 0),
    "location": "Info > RenderEngine",
    "warning": "Still under development", # used for warning icon and text in addons panel
    "category": "Render"}

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
        green_rect = [[0.0, 1.0, 0.0, 1.0]] * pixel_count

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
        blue_rect = [[0.0, 0.0, 1.0, 1.0]] * pixel_count

        # Here we write the pixel values to the RenderResult
        result = self.begin_result(0, 0, self.size_x, self.size_y)
        layer = result.layers[0]
        layer.rect = blue_rect
        self.end_result(result)

def register():
	# Register the RenderEngine
	bpy.utils.register_class(SORT_RENDERER)

	from bl_ui import properties_render
	properties_render.RENDER_PT_render.COMPAT_ENGINES.add('sort_renderer')
	del properties_render

	from bl_ui import properties_material
	properties_material.MATERIAL_PT_preview.COMPAT_ENGINES.add('sort_renderer')
	del properties_material

def unregister():
	print("sort_destroy")