import bpy
import platform
from extensions_framework import util as efutil
from bpy.types import AddonPreferences
from bpy.props import StringProperty
from . import common

class SORTAddonPreferences(AddonPreferences):
    bl_idname = common.preference_bl_name
    # this must match the addon name
    install_path = StringProperty(
            name="Path to SORT binary",
            description='Path to SORT binary',
            subtype='DIR_PATH',
            default='',
            )

    install_path_debug = StringProperty(
            name='Path to SORT binary(debug)',
            description='Path to SORT binary(debug)',
            subtype='DIR_PATH',
            default='',
            )

    def draw(self, context):
        layout = self.layout
        layout.prop(self, "install_path")
        layout.prop(self, "install_path_debug")

def get_sort_dir(force_debug=False):
    addon_prefs = bpy.context.user_preferences.addons[common.preference_bl_name].preferences
    debug = bpy.context.scene.debug_prop
    return_path = addon_prefs.install_path
    if debug is True:
        return_path = addon_prefs.install_path_debug
    if force_debug:
        return_path = addon_prefs.install_path_debug
    return efutil.filesystem_path(return_path) + "/"

def get_sort_bin_path(force_debug=False):
    sort_bin_dir = get_sort_dir(force_debug)
    if platform.system() == 'Darwin':   # for Mac OS
        sort_bin_path = sort_bin_dir + "sort"
    elif platform.system() == 'Windows':    # for Windows
        sort_bin_path = sort_bin_dir + "sort.exe"
    elif platform.system() == "Linux":
        sort_bin_path = sort_bin_dir + "SORT"
    else:
        raise Exception("SORT is only supported on Windows, Ubuntu and Mac OS")
    return sort_bin_path

def get_immediate_dir(force_debug=False):
    sort_bin_dir = get_sort_dir(force_debug)
    immediate_dir = sort_bin_dir + 'blender_intermediate/'
    return immediate_dir

def get_immediate_res_dir(force_debug=False):
    return get_immediate_dir(force_debug) + 'res/'

def register():
    bpy.utils.register_class(SORTAddonPreferences)

def unregister():
    bpy.utils.unregister_class(SORTAddonPreferences)
