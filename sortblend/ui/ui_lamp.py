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
import bl_ui
from .. import SORTAddon
from extensions_framework import declarative_property_group

# attach customized properties in lamp
@SORTAddon.addon_register_class
class sort_lamp(declarative_property_group):
    ef_attach_to = ['Lamp']
    controls = []
    visibility = {}
    properties = []

@SORTAddon.addon_register_class
class sort_lamp_hemi(declarative_property_group):
    ef_attach_to = ['sort_lamp']
    controls = []
    properties = [{ 'type': 'string',
                    'subtype': 'FILE_PATH',
                    'attr': 'envmap_file',
                    'name': 'HDRI Map',
                    'description': 'EXR image to use for lighting (in latitude-longitude format)',
                    'default': '',
                    'save_in_preset': True }]

class SORTLampPanel(bl_ui.properties_data_lamp.DataButtonsPanel):
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "data"
    sort_lamp_type = ''
    COMPAT_ENGINES = {'SORT_RENDERER'}
    @classmethod
    def poll(cls, context):
        if cls.sort_lamp_type is '' :
            return super().poll(context) and context.scene.render.engine in cls.COMPAT_ENGINES 
        return super().poll(context) and context.lamp.type == cls.sort_lamp_type and context.scene.render.engine in cls.COMPAT_ENGINES

class LampPanel(SORTLampPanel, bpy.types.Panel):
    bl_label = 'Lamp Property'
    def draw(self, context):
        lamp = context.lamp
        if context.lamp is not None:
            self.layout.prop(lamp, "type", expand=True)
            self.layout.prop(lamp, "color")
            self.layout.prop(lamp, "energy")

class LampHemiPanel(SORTLampPanel, bpy.types.Panel):
    bl_label = 'Lamp Hemi Property'
    sort_lamp_type = 'HEMI'
    def draw(self, context):
        self.layout.prop(context.lamp.sort_lamp.sort_lamp_hemi, "envmap_file", text="HDRI file")

class LampAreaPanel(SORTLampPanel, bpy.types.Panel):
    bl_label = 'Lamp Area Property'
    sort_lamp_type = 'AREA'
    def draw(self, context):
        layout = self.layout
        lamp = context.lamp
        split = layout.split()
        col = split.column(align=True)
        col.prop(lamp, "shape", text="")
        sub = split.column(align=True)
        if lamp.shape == 'SQUARE':
            sub.prop(lamp, "size")
        elif lamp.shape == 'RECTANGLE':
            sub.prop(lamp, "size", text="Size X")
            sub.prop(lamp, "size_y", text="Size Y")

class LampSpotPanel(SORTLampPanel, bpy.types.Panel):
    bl_label = 'Lamp Spot Property'
    sort_lamp_type = 'SPOT'
    def draw(self, context):
        self.layout.prop( context.lamp , "spot_size" , text="Spot Light Range" )
        self.layout.prop( context.lamp , "spot_blend" , text="Spot Light Blend" )