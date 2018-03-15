import bpy
import os
import math
from . import exporter_common
from .. import common
from .. import utility

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
    pbrt_film = "Film \"image\"\n"
    pbrt_film += "\t\"integer xresolution\" [" + '%d'%xres + "]\n"
    pbrt_film += "\t\"integer yresolution\" [" + '%d'%yres + "]\n"
    pbrt_film += "\t\"string filename\" [ \"" + pbrt_file_name + ".exr\" ]\n\n"

    # generating camera information
    fov = math.degrees( bpy.data.cameras[0].angle )
    camera = exporter_common.getCamera(scene)
    pos, target, up = exporter_common.lookAt(camera)
    pbrt_camera = "LookAt \t" + utility.vec3tostr( pos ) + "\n"
    pbrt_camera += "       \t" + utility.vec3tostr( target ) + "\n"
    pbrt_camera += "       \t" + utility.vec3tostr( up ) + "\n"
    pbrt_camera += "Camera \t\"perspective\"\n"
    pbrt_camera += "       \t\"float fov\" [" + '%f'%fov + "]\n\n"

    # sampler information
    sample_count = scene.sampler_count_prop
    pbrt_sampler = "Sampler \"random\" \"integer pixelsamples\" " + '%d'%sample_count + "\n"

    # integrator
    pbrt_integrator = "Integrator \"path\"" + " \"integer maxdepth\" " + '%d'%scene.inte_max_recur_depth + "\n"

    file = open(pbrt_file_fullpath,'w')
    file.write( pbrt_film )
    file.write( pbrt_camera )
    file.write( pbrt_sampler )
    file.write( pbrt_integrator )
    file.close()