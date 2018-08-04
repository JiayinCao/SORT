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
import math
import mathutils
import subprocess
import os.path
import platform
import numpy as np
from math import degrees
from . import exporter_common
from extensions_framework import util as efutil

pbrt_process = None

# get the path of pbrt binary folder
def get_pbrt_dir():
    return_path = exporter_common.getPreference().pbrt_export_path
    if return_path is '':
        return ''
    if platform.system() == 'Windows':
        return return_path
    return efutil.filesystem_path(return_path) + "/"

# get the path of pbrt executable file
def get_pbrt_bin_path():
    pbrt_bin_dir = get_pbrt_dir()
    if platform.system() == 'Darwin':   # for Mac OS
        pbrt_bin_dir = pbrt_bin_dir + "pbrt"
    elif platform.system() == 'Windows':    # for Windows
        pbrt_bin_dir = pbrt_bin_dir + "pbrt.exe"
    elif platform.system() == "Linux":
        pbrt_bin_dir = pbrt_bin_dir + "pbrt"
    else:
        raise Exception("PBRT is only supported on Windows, Ubuntu and Mac OS")
    return pbrt_bin_dir

# get camera data, to be merged with the above function
def lookAtPbrt(camera):
    # it seems that the matrix return here is the inverse of view matrix.
    ori_matrix = camera.matrix_world.copy()
    # get the transpose matrix
    matrix = ori_matrix.transposed()
    pos = matrix[3]             # get eye position
    forwards = -matrix[2]       # get forward direction

    # get focal distance for DOF effect
    if camera.data.dof_object is not None:
        focal_object = camera.data.dof_object
        fo_mat = focal_object.matrix_world
        delta = fo_mat.to_translation() - pos.to_3d()
        focal_distance = delta.dot(forwards)
    else:
        focal_distance = max( camera.data.dof_distance , 0.01 )
    scaled_forward = mathutils.Vector((focal_distance * forwards[0], focal_distance * forwards[1], focal_distance * forwards[2] , 0.0))
    # viewing target
    target = (pos + scaled_forward)
    # up direction
    up = matrix[1]
    return (pos, target, up)

# export blender information
def export_blender(scene, force_debug=False):
    node = export_scene(scene)
    export_material(scene)
    export_light(scene)
    pbrt_file_fullpath = export_pbrt_file(scene,node)

    # start rendering process first
    pbrt_bin_path = get_pbrt_bin_path()
    pbrt_bin_dir = get_pbrt_dir()

    if os.path.isfile(pbrt_bin_path) is False:
        print("Can't find pbrt executable file.")
        return

    # execute binary
    cmd_argument = [pbrt_bin_path]
    cmd_argument.append(pbrt_file_fullpath)
    global pbrt_process
    pbrt_process = subprocess.Popen(cmd_argument,cwd=pbrt_bin_dir)

# check if the process is still executing
def is_pbrt_executing():
    global pbrt_process
    if pbrt_process is None:
        return False
    return subprocess.Popen.poll(pbrt_process) is None

# shutdown pbrt process
def shutdown_pbrt():
    global pbrt_process
    subprocess.Popen.terminate(pbrt_process)
    pbrt_process = None

# get pbrt output file name
def get_pbrt_filename():
    pbrt_file_path = get_pbrt_dir()
    pbrt_file_name = exporter_common.getEditedFileName()
    return pbrt_file_path + pbrt_file_name + ".exr"

# export pbrt file
def export_pbrt_file(scene, node):
    # Get the path to save pbrt scene
    pbrt_file_path = exporter_common.getPreference().pbrt_export_path
    pbrt_file_name = exporter_common.getEditedFileName()
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
    pos, target, up = lookAtPbrt(camera)
    pbrt_camera = "Scale -1 1 1 \n"
    pbrt_camera += "LookAt \t" + exporter_common.vec3tostr( pos ) + "\n"
    pbrt_camera += "       \t" + exporter_common.vec3tostr( target ) + "\n"
    pbrt_camera += "       \t" + exporter_common.vec3tostr( up ) + "\n"
    pbrt_camera += "Camera \t\"perspective\"\n"
    pbrt_camera += "       \t\"float fov\" [" + '%f'%fov + "]\n\n"

    # sampler information
    sample_count = scene.sampler_count_prop
    pbrt_sampler = "Sampler \"random\" \"integer pixelsamples\" " + '%d'%sample_count + "\n"

    # integrator
    pbrt_integrator = "Integrator \"path\"" + " \"integer maxdepth\" " + '%d'%scene.inte_max_recur_depth + "\n\n"

    file = open(pbrt_file_fullpath,'w')
    file.write( pbrt_film )
    file.write( pbrt_camera )
    file.write( pbrt_sampler )
    file.write( pbrt_integrator )
    file.write( "WorldBegin\n" )
    file.write( "Include \"lights.pbrt\"\n" )
    file.write( "Include \"materials.pbrt\"\n" )
    for n in node:
        file.write( "Include \"" + n + ".pbrt\"\n" )
    file.write( "WorldEnd\n" )
    file.close()

    return pbrt_file_fullpath

# export scene
def export_scene(scene):
    ret = []
    for node in exporter_common.getMeshList(scene):
        export_mesh(node)
        ret.append(node.name)
    return ret;

def export_light(scene):
    pbrt_file_path = exporter_common.getPreference().pbrt_export_path
    pbrt_light_file_name = pbrt_file_path + "lights.pbrt"

    file = open(pbrt_light_file_name,'w')

    pbrt_lights = ''
    for ob in exporter_common.getLightList(scene):
        lamp = ob.data
        world_matrix = ob.matrix_world
        file.write( "AttributeBegin\n" )
        file.write( "Transform [" + exporter_common.matrixtostr( world_matrix.transposed() ) + "]\n" )
        if lamp.type == 'SUN':
            point_from = [0,1,0]
            point_to = [0,0,0]
            str = "LightSource \"distant\" \n"
            str += "\"rgb L\" [ %f %f %f ] \n"%(lamp.color[0],lamp.color[1],lamp.color[2])
            str += "\"point from\" [ %f %f %f ] \n"%(point_from[0],point_from[2],point_from[1])
            str += "\"point to\" [ %f %f %f ] \n"%(point_to[0],point_to[2],point_to[1])
            str += "\"rgb scale\" [ %f %f %f ] \n"%(lamp.energy,lamp.energy,lamp.energy)
            file.write(str)
        elif lamp.type == 'POINT':
            point_from = [0,0,0]
            str = "LightSource \"point\" \n"
            str += "\"rgb I\" [ %f %f %f ] \n"%(lamp.color[0],lamp.color[1],lamp.color[2])
            str += "\"point from\" [ %f %f %f ] \n"%(point_from[0],point_from[2],point_from[1])
            str += "\"rgb scale\" [ %f %f %f ] \n"%(lamp.energy,lamp.energy,lamp.energy)
            file.write(str)
        elif lamp.type == 'SPOT':
            point_from = [0,1,0]
            point_to = [0,0,0]
            str = "LightSource \"spot\" "
            str += "\"rgb I\" [ %f %f %f ] \n"%(lamp.color[0],lamp.color[1],lamp.color[2])
            str += "\"point from\" [ %f %f %f ] \n"%(point_from[0],point_from[2],point_from[1])
            str += "\"point to\" [ %f %f %f] \n"%(point_to[0],point_to[2],point_to[1])
            str += "\"rgb scale\" [ %f %f %f ] \n"%(lamp.energy,lamp.energy,lamp.energy)
            str += "\"float coneangle\" [ %f ] \n"%(degrees(lamp.spot_size*0.5))
            str += "\"float conedeltaangle\" [ %f ] \n"%(degrees(lamp.spot_size * lamp.spot_blend * 0.5))
            file.write(str)
        elif lamp.type == 'AREA':
            halfSizeX = lamp.size / 2
            halfSizeY = lamp.size_y / 2
            if lamp.shape == 'SQUARE':
                halfSizeY = halfSizeX
            light_spectrum = np.array(lamp.color[:])
            light_spectrum *= lamp.energy

            str = "AreaLightSource \"diffuse\" \"rgb L\" [%f %f %f] \n"%(light_spectrum[0],light_spectrum[1],light_spectrum[2])
            str += "Material \"matte\" \"rgb Kd\" [ 0.0 0.0 0.0 ]\n"
            str += "Shape \"trianglemesh\"\n"
            str += "\"integer indices\" [0 2 1 0 3 2]\n"
            str += "\"point P\" [ %f %f 0   %f %f 0   %f %f 0   %f %f 0 ]\n"%(-halfSizeX,-halfSizeY,halfSizeX,-halfSizeY,halfSizeX,halfSizeY,-halfSizeX,halfSizeY)
            file.write(str)
        elif lamp.type == 'HEMI':
            light_spectrum = np.array(lamp.color[:])
            light_spectrum *= lamp.energy
            str = "LightSource \"infinite\" "
            str += "\"rgb L\" [ %f %f %f ] \n"%(light_spectrum[0],light_spectrum[1],light_spectrum[2])
            str += "\"string mapname\" \"%s\" \n"%bpy.path.abspath(lamp.sort_lamp.sort_lamp_hemi.envmap_file).replace( '\\' , '/' )
            file.write(str)
        file.write( "AttributeEnd\n" )

    file.close()


def export_material(scene):
    pbrt_file_path = exporter_common.getPreference().pbrt_export_path
    pbrt_material_file_name = pbrt_file_path + "materials.pbrt"

    print( "Exporting pbrt file for material: " , pbrt_material_file_name )
    file = open( pbrt_material_file_name , 'w' )

    for material in exporter_common.getMaterialList(scene):
        ntree = bpy.data.node_groups[material.sort_material.sortnodetree]

        # find the output node, duplicated code, to be cleaned
        def find_output_node(material):
            if material and material.sort_material and material.sort_material.sortnodetree:
                ntree = bpy.data.node_groups[material.sort_material.sortnodetree]
                for node in ntree.nodes:
                    if getattr(node, "bl_idname", None) == 'SORTNodeOutput':
                        return node
            return None

        output_node = find_output_node(material)
        if output_node is None:
            continue

        if len(output_node.inputs) == 0:
            return

        print( 'Exporting material: ' + material.name )

        file.write( "MakeNamedMaterial \"" + material.name + "\"\n" )
        def socket_node_input(nt, socket):
            return next((l.from_node for l in nt.links if l.to_socket == socket), None)
        nput_node = socket_node_input(ntree, output_node.inputs[0])
        output_pbrt_type = lambda t : file.write( "  \"string type\" \"" + t + "\"\n" )
        output_pbrt_prop = lambda n , t , v : file.write( "  \"" + t + " " + n + "\" [" + v + "]\n")
        nput_node.export_pbrt(output_pbrt_type , output_pbrt_prop)
        file.write( '\n' )

    file.close()


def export_mesh(node):
    pbrt_file_path = exporter_common.getPreference().pbrt_export_path
    pbrt_geometry_file_name = pbrt_file_path + node.name + ".pbrt"

    print( "Exporting pbrt file for geometry: " , pbrt_geometry_file_name )
    file = open( pbrt_geometry_file_name , 'w' )

    mesh = node.data

    # begin attribute
    file.write( "AttributeBegin\n" )

    # setup material
    materials = mesh.materials[:]
    material_names = [m.name if m else None for m in materials]

    # avoid bad index errors
    if not materials:
        materials = [None]
        material_names = ["None"]
    file.write( "NamedMaterial \"" + material_names[0] + "\"\n" )

    # transform
    file.write( "Transform [" + exporter_common.matrixtostr( node.matrix_world.transposed() ) + "]\n" )

    # output triangle mesh
    file.write( "Shape \"trianglemesh\"\n")

    # output vertex buffer
    file.write( '\"point P\" [' )
    for v in mesh.vertices:
        file.write( exporter_common.vec3tostr( v.co ) + " " )
    file.write( "]\n" )

    file.write( "\"normal N\" [" )
    mesh.calc_normals_split()
    normals = [""] * len( mesh.vertices )
    for poly in mesh.polygons:
        for loop_index in poly.loop_indices:
            id = mesh.loops[loop_index].vertex_index
            normals[id] = exporter_common.vec3tostr( mesh.loops[loop_index].normal )
    file.write( " ".join( normals ) )
    file.write( "]\n" )

    faceuv = len(mesh.uv_textures) > 0
    if faceuv:
        uv_layer = mesh.uv_layers.active.data[:]
        uvs = [""] * len( mesh.vertices )

        file.write( "\"float st\" [" )
        for poly in mesh.polygons:
            for loop_index in poly.loop_indices:
                id = mesh.loops[loop_index].vertex_index
                uv = uv_layer[loop_index].uv
                uvs[id] = " %f %f"%(uv[0],uv[1])
        file.write( " ".join( uvs ) )
        file.write( "]\n" )

    # output index buffer
    file.write( "\"integer indices\" [" )
    for p in mesh.polygons:
        if len(p.vertices) == 3:
            file.write( "%d %d %d " %( p.vertices[0] , p.vertices[1] , p.vertices[2] ) )
        elif len(p.vertices) == 4:
            file.write( "%d %d %d %d %d %d " % (p.vertices[0],p.vertices[1],p.vertices[2],p.vertices[0],p.vertices[2],p.vertices[3]))
    file.write( "]\n" )

    # end attribute
    file.write( "AttributeEnd\n" )

    file.close()
