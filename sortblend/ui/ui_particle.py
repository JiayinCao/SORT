#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
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

# attach customized properties in particles
@SORTAddon.addon_register_class
class sort_particle(declarative_property_group):
    ef_attach_to = ['ParticleSettings']
    controls = []
    visibility = {}
    properties = []

@SORTAddon.addon_register_class
class sort_particle_width(declarative_property_group):
    ef_attach_to = ['sort_particle']
    controls = []
    properties = [{ 'type': 'float',
                    'attr': 'width_tip',
                    'name': 'Width of hair (tip)',
                    'description': 'Width at the tip of hair',
                    'default': 0.00,
                    'min': 0.0,
                    'soft_min': 0.0,
                    'max': 1e3,
                    'soft_max': 1e3,
                    'save_in_preset': True },
                    { 'type': 'float',
                    'attr': 'width_bottom',
                    'name': 'Width of hair (bottom)',
                    'description': 'Width at the bottom of hair',
                    'default': 0.01,
                    'min': 0.0,
                    'soft_min': 0.0,
                    'max': 1e3,
                    'soft_max': 1e3,
                    'save_in_preset': True }]

class SORTParticlePanel(bl_ui.properties_particle.ParticleButtonsPanel):
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "particle"
    COMPAT_ENGINES = {'SORT_RENDERER'}
    @classmethod
    def register(cls):
        bl_ui.properties_particle.PARTICLE_PT_boidbrain.COMPAT_ENGINES.add('SORT_RENDERER')
        bl_ui.properties_particle.PARTICLE_PT_cache.COMPAT_ENGINES.add('SORT_RENDERER')
        bl_ui.properties_particle.PARTICLE_PT_children.COMPAT_ENGINES.add('SORT_RENDERER')
        bl_ui.properties_particle.PARTICLE_PT_context_particles.COMPAT_ENGINES.add('SORT_RENDERER')
        bl_ui.properties_particle.PARTICLE_PT_custom_props.COMPAT_ENGINES.add('SORT_RENDERER')
        bl_ui.properties_particle.PARTICLE_PT_draw.COMPAT_ENGINES.add('SORT_RENDERER')
        bl_ui.properties_particle.PARTICLE_PT_emission.COMPAT_ENGINES.add('SORT_RENDERER')
        bl_ui.properties_particle.PARTICLE_PT_field_weights.COMPAT_ENGINES.add('SORT_RENDERER')
        bl_ui.properties_particle.PARTICLE_PT_force_fields.COMPAT_ENGINES.add('SORT_RENDERER')
        bl_ui.properties_particle.PARTICLE_PT_hair_dynamics.COMPAT_ENGINES.add('SORT_RENDERER')
        bl_ui.properties_particle.PARTICLE_PT_physics.COMPAT_ENGINES.add('SORT_RENDERER')
        bl_ui.properties_particle.PARTICLE_PT_render.COMPAT_ENGINES.add('SORT_RENDERER')
        bl_ui.properties_particle.PARTICLE_PT_rotation.COMPAT_ENGINES.add('SORT_RENDERER')
        bl_ui.properties_particle.PARTICLE_PT_hair_dynamics.COMPAT_ENGINES.add('SORT_RENDERER')
        bl_ui.properties_particle.PARTICLE_PT_velocity.COMPAT_ENGINES.add('SORT_RENDERER')
        bl_ui.properties_particle.PARTICLE_PT_vertexgroups.COMPAT_ENGINES.add('SORT_RENDERER')

    @classmethod
    def poll(cls, context):
        psys = context.particle_system
        engine = context.scene.render.engine
        if psys is None:
            return False
        if psys.settings is None:
            return False
        return psys.settings.type == 'HAIR' and (engine in cls.COMPAT_ENGINES)

class HairPanel(SORTParticlePanel, bpy.types.Panel):
    bl_label = 'SORT Hair Property'
    def draw(self, context):
        layout = self.layout
        ps = context.particle_settings
        layout.prop(ps.sort_particle.sort_particle_width, "width_tip")
        layout.prop(ps.sort_particle.sort_particle_width, "width_bottom")