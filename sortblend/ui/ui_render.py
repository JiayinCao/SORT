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
import os
import bl_ui
import platform
import subprocess
from ..exporter import sort_exporter
from ..exporter import pbrt_exporter

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
    COMPAT_ENGINES = {'SORT_RENDERER'}

    @classmethod
    def register(cls):
        bl_ui.properties_render.RENDER_PT_render.COMPAT_ENGINES.add('SORT_RENDERER')
        bl_ui.properties_render.RENDER_PT_dimensions.COMPAT_ENGINES.add('SORT_RENDERER')

    @classmethod
    def poll(cls, context):
        return context.scene.render.engine in cls.COMPAT_ENGINES

class IntegratorPanel(SORTRenderPanel,bpy.types.Panel):
    bl_label = 'Renderer'

    # Integrator type
    integrator_types = [ ("pt", "Path Tracing", "", 1),
                         ("bdpt", "Bidirectional Path Tracing", "", 2),
                         ("lt", "Light Tracing", "", 3),
                         ("ir", "Instant Radiosity", "", 4),
                         ("ao", "Ambient Occlusion", "", 5),
                         ("direct", "Direct Lighting", "", 6),
                         ("whitted", "Whitted", "", 7) ]
    bpy.types.Scene.integrator_type_prop = bpy.props.EnumProperty(items=integrator_types, name='Integrator')

    # Accelerator type
    accelerator_types = [ ("bvh", "Bounding Volume Hierarchy", "", 1),
                          ("kd_tree", "SAH KDTree", "", 2),
                          ("uniform_grid", "Uniform Grid", "", 3),
                          ("octree" , "OcTree" , "" , 4),
                          ("bruteforce", "No Accelerator", "", 5) ]
    bpy.types.Scene.accelerator_type_prop = bpy.props.EnumProperty(items=accelerator_types, name='Accelerator')

    # general integrator parameters
    bpy.types.Scene.inte_max_recur_depth = bpy.props.IntProperty(name='Maximum Recursive Depth', default=16, min=1)

    # ao integrator parameters
    bpy.types.Scene.ao_max_dist = bpy.props.FloatProperty(name='Maximum Distance', default=3.0, min=0.01)

    # instant radiosity parameters
    bpy.types.Scene.ir_light_path_set_num = bpy.props.IntProperty(name='Light Path Set Num', default=1, min=1)
    bpy.types.Scene.ir_light_path_num = bpy.props.IntProperty(name='Light Path Num', default=64, min=1)
    bpy.types.Scene.ir_min_dist = bpy.props.FloatProperty(name='Minimum Distance', default=1.0, min=0.0)

    # bidirectional path tracing parameters
    bpy.types.Scene.bdpt_mis = bpy.props.BoolProperty(name='Multiple Importance Sampling', default=True)

    def draw(self, context):
        self.layout.prop(context.scene,"integrator_type_prop")
        integrator_type = context.scene.integrator_type_prop
        if integrator_type != "whitted" and integrator_type != "direct" and integrator_type != "ao":
            self.layout.prop(context.scene,"inte_max_recur_depth")
        if integrator_type == "ao":
            self.layout.prop(context.scene,"ao_max_dist")
        if integrator_type == "bdpt":
            self.layout.prop(context.scene,"bdpt_mis")
        if integrator_type == "ir":
            self.layout.prop(context.scene,"ir_light_path_set_num")
            self.layout.prop(context.scene,"ir_light_path_num")
            self.layout.prop(context.scene, "ir_min_dist")
        self.layout.prop(context.scene,"accelerator_type_prop")

class MultiThreadPanel(SORTRenderPanel, bpy.types.Panel):
    bl_label = 'MultiThread'
    bpy.types.Scene.thread_num_prop = bpy.props.IntProperty(name='Thread Num', default=8, min=1, max=16)
    def draw(self, context):
        self.layout.prop(context.scene,"thread_num_prop")

class SamplerPanel(SORTRenderPanel, bpy.types.Panel):
    bl_label = 'Sample'
    sampler_types = [ ("stratified", "Stratified", "", 3), ("random", "Random", "", 2), ("regular", "Uniform", "", 1) ]
    bpy.types.Scene.sampler_type_prop = bpy.props.EnumProperty(items=sampler_types, name='Type')
    bpy.types.Scene.sampler_count_prop = bpy.props.IntProperty(name='Count',default=1, min=1)
    def draw(self, context):
        self.layout.prop(context.scene,"sampler_type_prop")
        self.layout.prop(context.scene,"sampler_count_prop")

# export debug scene
class SORT_export_debug_scene(bpy.types.Operator):
    bl_idname = "sort.export_debug_scene"
    bl_label = "Export SORT Scene"

    def execute(self, context):
        sort_exporter.export_blender(context.scene,True)
        return {'FINISHED'}


class SORT_open_log(bpy.types.Operator):
    bl_idname = "sort.open_log"
    bl_label = "Open Log"
    def execute(self, context):
        logfile = sort_exporter.get_sort_dir() + "log.txt"
        OpenFile( logfile )
        return {'FINISHED'}


class SORT_openfolder(bpy.types.Operator):
    bl_idname = "sort.openfolder_sort"
    bl_label = "Open SORT folder"
    
    def execute(self, context):
        OpenFolder( sort_exporter.get_sort_dir() )
        return {'FINISHED'}


class DebugPanel(SORTRenderPanel, bpy.types.Panel):
    bl_label = 'DebugPanel'
    def draw(self, context):
        self.layout.operator("sort.export_debug_scene")
        split = self.layout.split()
        left = split.column(align=True)
        left.operator("sort.open_log")
        right = split.column(align=True)
        right.operator("sort.openfolder_sort")


export_pbrt_lable = "Render in PBRT"
pbrt_running = False
class PBRT_export_scene(bpy.types.Operator):
    bl_idname = "sort.export_pbrt_scene"
    bl_label = "Export PBRT scene"

    def modal(self, context, event):
        if event.type == 'TIMER':
            # this is a very hacky way to update the UI, somehow this line will invalidate the window forcing a redraw
            color = context.user_preferences.themes[0].view_3d.space.gradients.high_gradient
            color.s = color.s
            if pbrt_exporter.is_pbrt_executing() is False:
                global export_pbrt_lable
                export_pbrt_lable = "Render in PBRT"
                # chexk the result automatically
                if pbrt_running is True:
                    pbrt_file_name = pbrt_exporter.get_pbrt_filename()
                    OpenFile( pbrt_file_name )
                # remove timer
                wm = context.window_manager
                wm.event_timer_remove(self._timer)
                return {'CANCELLED'}
        return {'PASS_THROUGH'}

    def execute(self, context):
        global export_pbrt_lable
        global pbrt_running
        if pbrt_exporter.is_pbrt_executing() is False:
            pbrt_running = True
            wm = context.window_manager
            self._timer = wm.event_timer_add(0.01, context.window)
            wm.modal_handler_add(self)
            export_pbrt_lable = "Shutdown PBRT"
            pbrt_exporter.export_blender(context.scene,True)
        else:
            pbrt_running = False
            export_pbrt_lable = "Render in PBRT"
            pbrt_exporter.shutdown_pbrt()
            return {'CANCELLED'}
        return {'RUNNING_MODAL'}


class PBRT_checkresult(bpy.types.Operator):
    bl_idname = "sort.checkresult_pbrt"
    bl_label = "Check PBRT Result"
    def execute(self, context):
        OpenFile( pbrt_exporter.get_pbrt_filename() )
        return {'FINISHED'}

class PBRT_openfolder(bpy.types.Operator):
    bl_idname = "sort.openfolder_pbrt"
    bl_label = "Open PBRT folder"
    def execute(self, context):
        OpenFolder( pbrt_exporter.get_pbrt_dir() )
        return {'FINISHED'}

class PBRTDebugPanel(SORTRenderPanel, bpy.types.Panel):
    bl_label = 'PBRT Debug Panel'

    @classmethod
    def poll(cls,context):
        return pbrt_exporter.get_pbrt_dir() is not '' and super().poll(context)

    def draw(self, context):
        self.layout.operator("sort.export_pbrt_scene",text=export_pbrt_lable)
        split = self.layout.split()
        left = split.column(align=True)
        left.operator("sort.checkresult_pbrt")
        right = split.column(align=True)
        right.operator("sort.openfolder_pbrt")
