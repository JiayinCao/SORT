import bpy
from extensions_framework import util as efutil
from bpy.types import AddonPreferences
from bpy.props import StringProperty

class SORTAddonPreferences(AddonPreferences):
    bl_idname = __package__
    # this must match the addon name
    install_path = StringProperty(
            name="Path to SORT binary",
            description='Path to SORT binary',
            subtype='DIR_PATH',
            default='',
            )

    def draw(self, context):
        layout = self.layout
        layout.prop(self, "install_path")

def get_sort_path():
    addon_prefs = bpy.context.user_preferences.addons[__package__].preferences
    return efutil.filesystem_path(addon_prefs.install_path) + "/"

def register():
    bpy.utils.register_class(SORTAddonPreferences)

def unregister():
    bpy.utils.unregister_class(SORTAddonPreferences)