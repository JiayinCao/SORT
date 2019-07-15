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
class SORTHair(bpy.types.PropertyGroup):
    hair_tip : bpy.props.FloatProperty(name='Hair Tip' , default=0.0 , min=0.0 )
    hair_bottom : bpy.props.FloatProperty(name='Hair Bottom' , default=0.1 , min=0.0 )

    @classmethod
    def register(cls):
        bpy.types.ParticleSettings.sort_hair = bpy.props.PointerProperty(name="SORT Hair Setting", type=cls)
    @classmethod
    def unregister(cls):
        del bpy.types.Scene.sort_hair

@base.register_class
class SORT_PT_HairSettingPanel(bpy.types.Panel):
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "particle"
    bl_label = 'SORT Hair Setting'

    COMPAT_ENGINES = {'SORT'}
    @classmethod
    def poll(cls, context):
        return context.scene.render.engine in cls.COMPAT_ENGINES

    def draw(self, context):
        hair = context.particle_settings.sort_hair
        self.layout.prop(hair, "hair_tip")
        self.layout.prop(hair, "hair_bottom")