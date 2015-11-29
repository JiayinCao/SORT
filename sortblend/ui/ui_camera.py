import bpy
import bl_ui
from .. import common
from .. import SORTAddon
from extensions_framework import declarative_property_group

# attach customized properties in lamp
@SORTAddon.addon_register_class
class sort_camera(declarative_property_group):
    ef_attach_to = ['Camera']

    controls = []
    visibility = {}
    properties = []

@SORTAddon.addon_register_class
class sort_camera_lens(declarative_property_group):
    ef_attach_to = ['sort_camera']

    controls = []

    properties = [
        {
            'type': 'float',
            'attr': 'lens_size',
            'name': 'Size of Camera Lens',
            'description': 'The size of lens in camera',
            'default': 1.0,
            'min': 1e-3,
            'soft_min': 1e-3,
            'max': 1e3,
            'soft_max': 1e3,
            'save_in_preset': True
        },
    ]

class SORTCameraPanel(bl_ui.properties_data_camera.CameraButtonsPanel):
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "data"
    COMPAT_ENGINES = {common.default_bl_name}

    @classmethod
    def poll(cls, context):
        rd = context.scene.render
        return super().poll(context) and rd.engine in cls.COMPAT_ENGINES

class CameraDOFPanel(SORTCameraPanel, bpy.types.Panel):
    bl_label = 'Camera Depth of Field'

    def draw(self, context):
        layout = self.layout
        camera = context.camera
        layout.prop(camera, "dof_object")
        row = layout.row()
        row.active = ( camera.dof_object == None )
        row.prop(camera, "dof_distance")
        layout.prop(camera.sort_camera.sort_camera_lens, "lens_size")
        pass