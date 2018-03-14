import bpy
import os
import shutil
import mathutils
import math
import numpy as np
from . import preference
from . import common
from . import nodes
from . import utility
import xml.etree.cElementTree as ET

# export blender information
def export_blender(scene, force_debug=False):
    # Get the path to save pbrt scene
    addon_prefs = bpy.context.user_preferences.addons[common.preference_bl_name].preferences
    pbrt_file_path = addon_prefs.pbrt_export_path
    pbrt_file_name = os.path.splitext(os.path.basename(bpy.data.filepath))[0]
    if not pbrt_file_name:
        pbrt_file_name = 'default_scene'
    pbrt_file_fullpath = pbrt_file_path + pbrt_file_name + ".pbrt"

    print( 'Exporting PBRT Scene :' , pbrt_file_fullpath )

    # generating the film header
    xres = scene.render.resolution_x * scene.render.resolution_percentage / 100
    yres = scene.render.resolution_y * scene.render.resolution_percentage / 100
    film = "Film \"image\"\n";
    film += "\t\"integer xresolution\" [" + '%d'%xres + "]\n";
    film += "\t\"integer yresolution\" [" + '%d'%yres + "]\n";
    film += "\t\"string filename\" [ \"" + pbrt_file_name + ".exr\" ]"

    # generating camera information
    

    file = open(pbrt_file_fullpath,'w')
    file.write( film )
    file.close()