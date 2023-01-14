#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.
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
from bl_ui import properties_particle

# attach customized properties to particles
@base.register_class
class SORTParticleData(bpy.types.PropertyGroup):
    fur_tip : bpy.props.FloatProperty( name='Fur Tip', default=0.0)
    fur_bottom : bpy.props.FloatProperty( name='Fur Bottom', default=0.01)
    @classmethod
    def register(cls):
        bpy.types.ParticleSettings.sort_data = bpy.props.PointerProperty(name="SORT Data", type=cls)
    @classmethod
    def unregister(cls):
        del bpy.types.ParticleSettings.sort_data

class SORTParticlePanel(properties_particle.ParticleButtonsPanel):
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "particle"
    COMPAT_ENGINES = {'SORT'}

    @classmethod
    def poll(cls, context):
        psys = context.particle_system
        engine = context.scene.render.engine
        if psys is None:
            return False
        if psys.settings is None:
            return False
        return psys.settings.type == 'HAIR' and (engine in cls.COMPAT_ENGINES)

@base.register_class
class HAIR_PT_SORTSettingPanel(SORTParticlePanel, bpy.types.Panel):
    bl_label = 'SORT Hair Property'
    def draw(self, context):
        layout = self.layout
        ps = context.particle_settings
        layout.prop(ps.sort_data, "fur_tip")
        layout.prop(ps.sort_data, "fur_bottom")
