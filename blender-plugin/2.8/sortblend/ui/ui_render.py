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
import os
import platform
import subprocess
from .. import exporter
from .. import base

# attach customized properties in particles
@base.register_class
class SORTRenderData(bpy.types.PropertyGroup):
    #------------------------------------------------------------------------------------#
    #                                 Renderer Settings                                  #
    #------------------------------------------------------------------------------------#
    # Integrator type
    integrator_types = [ ("PathTracing", "Path Tracing", "", 1),
                         ("BidirPathTracing", "Bidirectional Path Tracing", "", 2),
                         ("LightTracing", "Light Tracing", "", 3),
                         ("InstantRadiosity", "Instant Radiosity", "", 4),
                         ("AmbientOcclusion", "Ambient Occlusion", "", 5),
                         ("DirectLight", "Direct Lighting", "", 6),
                         ("WhittedRT", "Whitted", "", 7) ]
    integrator_type_prop : bpy.props.EnumProperty(items=integrator_types, name='Accelerator')

    # general integrator parameters
    inte_max_recur_depth : bpy.props.IntProperty(name='Maximum Recursive Depth', default=16, min=1)

    # maxmum bounces supported in BSSRDF, exceeding the threshold will result in replacing BSSRDF with Lambert
    max_bssrdf_bounces : bpy.props.IntProperty(name='Maximum Bounces in SSS path', default=4, min=1)

    # ao integrator parameters
    ao_max_dist : bpy.props.FloatProperty(name='Maximum Distance', default=3.0, min=0.01)

    # instant radiosity parameters
    ir_light_path_set_num : bpy.props.IntProperty(name='Light Path Set Num', default=1, min=1)
    ir_light_path_num : bpy.props.IntProperty(name='Light Path Num', default=64, min=1)
    ir_min_dist : bpy.props.FloatProperty(name='Minimum Distance', default=1.0, min=0.0)

    # bidirectional path tracing parameters
    bdpt_mis : bpy.props.BoolProperty(name='Multiple Importance Sampling', default=True)

    #------------------------------------------------------------------------------------#
    #                              Spatial Accelerator Settings                          #
    #------------------------------------------------------------------------------------#
    # Accelerator type
    accelerator_types = [ ("bvh", "Bounding Volume Hierarchy", "", 1),
                          ("KDTree", "SAH KDTree", "", 2),
                          ("UniGrid", "Uniform Grid", "", 3),
                          ("OcTree" , "OcTree" , "" , 4),
                          ("bruteforce", "No Accelerator", "", 5) ]
    accelerator_type_prop : bpy.props.EnumProperty(items=accelerator_types, name='Accelerator')

    # bvh properties
    bvh_max_node_depth : bpy.props.IntProperty(name='Maximum Recursive Depth', default=28, min=8)
    bvh_max_pri_in_leaf : bpy.props.IntProperty(name='Maximum Primitives in Leaf Node.', default=8, min=8, max=64)

    # kdtree properties
    kdtree_max_node_depth : bpy.props.IntProperty(name='Maximum Recursive Depth', default=28, min=8)
    kdtree_max_pri_in_leaf : bpy.props.IntProperty(name='Maximum Primitives in Leaf Node.', default=8, min=8, max=64)

    # octree properties
    octree_max_node_depth : bpy.props.IntProperty(name='Maximum Recursive Depth', default=16, min=8)
    octree_max_pri_in_leaf : bpy.props.IntProperty(name='Maximum Primitives in Leaf Node.', default=16, min=8, max=64)

    #------------------------------------------------------------------------------------#
    #                                 Sampling Settings                                  #
    #------------------------------------------------------------------------------------#
    sampler_count_prop : bpy.props.IntProperty(name='Count',default=1, min=1)

    #------------------------------------------------------------------------------------#
    #                                 Threading Settings                                 #
    #------------------------------------------------------------------------------------#
    thread_num_prop : bpy.props.IntProperty(name='Thread Num', default=8, min=1, max=32)

    #------------------------------------------------------------------------------------#
    #                                 Debugging Settings                                 #
    #------------------------------------------------------------------------------------#
    detailedLog : bpy.props.BoolProperty( name='Output Detailed Output', default=False, description='Whether outputing detail log information in blender plugin.' )
    profilingEnabled : bpy.props.BoolProperty(name='Enable Profiling',default=False,description='Enabling profiling will have a big impact on performance, only use it for simple scene')
    allUseDefaultMaterial : bpy.props.BoolProperty(name='No Material',default=False,description='Disable all materials in SORT, use the default one.')
    
    @classmethod
    def register(cls):
        bpy.types.Scene.sort_data = bpy.props.PointerProperty(name="SORT Data", type=cls)
    @classmethod
    def unregister(cls):
        del bpy.types.Scene.sort_data

def OpenFile( filename ):
    if platform.system() == 'Darwin':     # for Mac OS
        os.system( "open \"%s\""%filename)
    elif platform.system() == 'Windows':  # for Windows
        os.system("\"%s\""%filename)
    elif platform.system() == "Linux":    # for linux
        os.system( "xdg-open \"%s\""%filename )

def OpenFolder( path ):
    if platform.system() == 'Darwin':     # for Mac OS
        subprocess.call(["open", "-R", path])
    elif platform.system() == 'Windows':  # for Windows
        subprocess.call("explorer \"%s\""%path)
    elif platform.system() == "Linux":    # for linux
        os.system( "xdg-open \"%s\""%path )

class SORTRenderPanel:
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "render"
    COMPAT_ENGINES = {'SORT'}

    @classmethod
    def poll(cls, context):
        return context.scene.render.engine in cls.COMPAT_ENGINES

@base.register_class
class RENDER_PT_IntegratorPanel(SORTRenderPanel,bpy.types.Panel):
    bl_label = 'Renderer'

    def draw(self, context):
        data = context.scene.sort_data
        self.layout.prop(data,"integrator_type_prop")
        integrator_type = data.integrator_type_prop
        if integrator_type != "WhittedRT" and integrator_type != "DirectLight" and integrator_type != "AmbientOcclusion":
            self.layout.prop(data,"inte_max_recur_depth")
        if integrator_type == "PathTracing":
            self.layout.prop(data,"max_bssrdf_bounces" )
        if integrator_type == "AmbientOcclusion":
            self.layout.prop(data,"ao_max_dist")
        if integrator_type == "BidirPathTracing":
            self.layout.prop(data,"bdpt_mis")
        if integrator_type == "InstantRadiosity":
            self.layout.prop(data,"ir_light_path_set_num")
            self.layout.prop(data,"ir_light_path_num")
            self.layout.prop(data, "ir_min_dist")

@base.register_class
class RENDER_PT_AcceleratorPanel(SORTRenderPanel,bpy.types.Panel):
    bl_label = 'Accelerator'
    def draw(self, context):
        data = context.scene.sort_data
        self.layout.prop(data,"accelerator_type_prop")
        accelerator_type = data.accelerator_type_prop
        if accelerator_type == "bvh":
            self.layout.prop(data,"bvh_max_node_depth")
            self.layout.prop(data,"bvh_max_pri_in_leaf")
        elif accelerator_type == "KDTree":
            self.layout.prop(data,"kdtree_max_node_depth")
            self.layout.prop(data,"kdtree_max_pri_in_leaf")
        elif accelerator_type == "OcTree":
            self.layout.prop(data,"octree_max_node_depth")
            self.layout.prop(data,"octree_max_pri_in_leaf")

@base.register_class
class RENDER_PT_MultiThreadPanel(SORTRenderPanel, bpy.types.Panel):
    bl_label = 'MultiThread'
    def draw(self, context):
        self.layout.prop(context.scene.sort_data,"thread_num_prop")

@base.register_class
class RENDER_PT_SamplerPanel(SORTRenderPanel, bpy.types.Panel):
    bl_label = 'Sample'
    def draw(self, context):
        self.layout.prop(context.scene.sort_data,"sampler_count_prop")

@base.register_class
class SORT_export_debug_scene(bpy.types.Operator):
    bl_idname = "sort.export_debug_scene"
    bl_label = "Export SORT Scene"
    def execute(self, context):
        depsgraph = context.evaluated_depsgraph_get()
        exporter.export_blender(depsgraph,True)
        return {'FINISHED'}

@base.register_class
class SORT_open_log(bpy.types.Operator):
    bl_idname = "sort.open_log"
    bl_label = "Open Log"
    def execute(self, context):
        logfile = exporter.get_sort_dir() + "log.txt"
        OpenFile( logfile )
        return {'FINISHED'}

@base.register_class
class SORT_openfolder(bpy.types.Operator):
    bl_idname = "sort.openfolder_sort"
    bl_label = "Open SORT folder"
    def execute(self, context):
        OpenFolder( exporter.get_sort_dir() )
        return {'FINISHED'}

@base.register_class
class RENDER_PT_DebugPanel(SORTRenderPanel, bpy.types.Panel):
    bl_label = 'DebugPanel'
    def draw(self, context):
        self.layout.operator("sort.export_debug_scene")
        split = self.layout.split()
        left = split.column(align=True)
        left.operator("sort.open_log")
        right = split.column(align=True)
        right.operator("sort.openfolder_sort")

        data = context.scene.sort_data
        self.layout.prop(data, "detailedLog")
        self.layout.prop(data, "profilingEnabled")
        self.layout.prop(data, "allUseDefaultMaterial")

