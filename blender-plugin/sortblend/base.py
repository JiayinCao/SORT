#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.
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

# a global container is used to keep all lambda function to register classes
REGISTRARS = []
def registrar(register, unregister, name=None):
    global REGISTRARS
    if name is None or not [True for _, _, n in REGISTRARS if n == name]:
        REGISTRARS.append((register, unregister, name))

# register a class in blender system, this function just
def register_class(cls):
    registrar(lambda: bpy.utils.register_class(cls), lambda: bpy.utils.unregister_class(cls), cls.__name__)
    return cls

# register some internal SORT compatible panels
def get_sort_compatible_panels():
    def is_panel_compatible(panel):
        compatible_panels = {
            'RENDER_PT_dimensions',
            'DATA_PT_lens',
            'DATA_PT_camera',
            'PARTICLE_PT_boidbrain',
            'PARTICLE_PT_cache',
            'PARTICLE_PT_children',
            'PARTICLE_PT_context_particles',
            'PARTICLE_PT_draw',
            'PARTICLE_PT_emission',
            'PARTICLE_PT_field_weights',
            'PARTICLE_PT_force_fields',
            'PARTICLE_PT_hair_dynamics',
            'PARTICLE_PT_physics',
            'PARTICLE_PT_render',
            'PARTICLE_PT_rotation',
            'PARTICLE_PT_velocity',
            'PARTICLE_PT_vertexgroups',
            'PHYSICS_PT_smoke',
            'PHYSICS_PT_smoke_settings',
            'PHYSICS_PT_smoke_settings_initial_velocity',
            'PHYSICS_PT_smoke_settings_particle_size',
            'PHYSICS_PT_smoke_behavior',
            'PHYSICS_PT_smoke_behavior_dissolve',
            'PHYSICS_PT_smoke_fire',
            'PHYSICS_PT_smoke_cache',
            'PHYSICS_PT_smoke_field_weights',
            'PHYSICS_PT_smoke_highres',
            'PHYSICS_PT_add',
            'PHYSICS_PT_field',
            'PHYSICS_PT_field_settings',
            'PHYSICS_PT_field_falloff',
        }

        return hasattr(panel, 'COMPAT_ENGINES') and 'BLENDER_RENDER' in panel.COMPAT_ENGINES and panel.__name__ in compatible_panels
    return [panel for panel in bpy.types.Panel.__subclasses__() if is_panel_compatible(panel)]

# trigger the real registering of all lambda function in the container
def register():
    for r, _, _ in REGISTRARS:
        r()

    for t in get_sort_compatible_panels():
        t.COMPAT_ENGINES.add('SORT')

# unregister everything already registered
def unregister():
    for t in get_sort_compatible_panels():
        t.COMPAT_ENGINES.remove('SORT')

    for _, u, _ in reversed(REGISTRARS):
        u()
