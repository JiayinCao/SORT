#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.
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
from .. import base

@base.register_class
class SORTHDRSky(bpy.types.PropertyGroup):
    hdr_image : bpy.props.PointerProperty(type=bpy.types.Image)
    @classmethod
    def register(cls):
        bpy.types.Scene.sort_hdr_sky = bpy.props.PointerProperty(name="SORT HDR Sky", type=cls)
    @classmethod
    def unregister(cls):
        del bpy.types.Scene.sort_hdr_sky

@base.register_class
class HDRSKY_PT_SORTPanel(bpy.types.Panel):
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "world"
    bl_label = 'HDR Sky'

    COMPAT_ENGINES = {'SORT'}
    @classmethod
    def poll(cls, context):
        return context.scene.render.engine in cls.COMPAT_ENGINES

    def draw(self, context):
        self.layout.template_ID(context.scene.sort_hdr_sky, 'hdr_image', open='image.open')