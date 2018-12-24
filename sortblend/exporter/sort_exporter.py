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
import mathutils
import shutil
import numpy as np
import platform
import tempfile
import bmesh
import time
from math import degrees
from . import exporter_common
from ..stream import stream
from extensions_framework import util as efutil

def get_sort_dir():
    return_path = exporter_common.getPreference().install_path
    if platform.system() == 'Windows':
        return return_path
    return efutil.filesystem_path(return_path) + "/"

def get_sort_bin_path():
    sort_bin_dir = get_sort_dir()
    if platform.system() == 'Darwin':   # for Mac OS
        sort_bin_path = sort_bin_dir + "sort_r"
    elif platform.system() == 'Windows':    # for Windows
        sort_bin_path = sort_bin_dir + "sort_r.exe"
    elif platform.system() == "Linux":
        sort_bin_path = sort_bin_dir + "sort_r"
    else:
        raise Exception("SORT is only supported on Windows, Ubuntu and Mac OS")
    return sort_bin_path

intermediate_dir = ''
def get_intermediate_dir(force_debug=False):
    global intermediate_dir
    return_path = intermediate_dir
    if force_debug is True:
        return_path = get_sort_dir()
    return efutil.filesystem_path(return_path) + "/"

def MatrixSortToBlender():
    from bpy_extras.io_utils import axis_conversion
    global_matrix = axis_conversion(to_forward='-Z',to_up='Y').to_4x4()
    global_matrix[2][0] *= -1.0
    global_matrix[2][1] *= -1.0
    global_matrix[2][2] *= -1.0
    return global_matrix

def MatrixBlenderToSort():
    from bpy_extras.io_utils import axis_conversion
    global_matrix = axis_conversion(to_forward='-Z',to_up='Y').to_4x4()
    global_matrix[2][0] *= -1.0
    global_matrix[2][1] *= -1.0
    global_matrix[2][2] *= -1.0
    return global_matrix

# get camera data
def lookAtSORT(camera):
    # it seems that the matrix return here is the inverse of view matrix.
    ori_matrix = MatrixBlenderToSort() * camera.matrix_world.copy()
    # get the transpose matrix
    matrix = ori_matrix.transposed()
    pos = matrix[3]             # get eye position
    forwards = -matrix[2]       # get forward direction

    # get focal distance for DOF effect
    if camera.data.dof_object is not None:
        focal_object = camera.data.dof_object
        fo_mat = MatrixBlenderToSort() * focal_object.matrix_world
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
    exporter_common.setScene(scene)

    # create immediate file path
    sort_resource_path = create_path(scene, force_debug)

    sort_config_file = sort_resource_path + 'scene.sort'
    fs = stream.FileStream( sort_config_file )
    
    exporter_common.log("Exporting sort file %s" % sort_config_file)

    # export global material settings
    current_time = time.time()
    exporter_common.log("Exporting global configuration.")
    export_global_config(scene, fs, sort_resource_path)
    exporter_common.log("Exported configuration %.2f" % (time.time() - current_time))
    current_time = time.time()

    # export material
    exporter_common.log("Exporting materials.")
    export_material(scene, fs)
    exporter_common.log("Exported materials %.2f" % (time.time() - current_time))
    current_time = time.time()

    # export scene
    exporter_common.log("Exporting scene.")
    export_scene(scene, fs)
    exporter_common.log("Exported scene %.2f" % (time.time() - current_time))
    current_time = time.time()

# clear old data and create new path
def create_path(scene, force_debug):
    global intermediate_dir
    intermediate_dir = tempfile.mkdtemp(suffix='w')
    # get immediate directory
    output_dir = get_intermediate_dir(force_debug)
    
    # create one if there is no such directory
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)
    return output_dir

# export scene
def export_scene(scene, fs):
    # camera node
    camera = exporter_common.getCamera(scene)
    pos, target, up = lookAtSORT(camera)
    sensor_w = bpy.data.cameras[0].sensor_width
    sensor_h = bpy.data.cameras[0].sensor_height
    sensor_fit = 0.0 # auto
    sfit = bpy.data.cameras[0].sensor_fit
    if sfit == 'VERTICAL':
        sensor_fit = 2.0
    elif sfit == 'HORIZONTAL':
        sensor_fit = 1.0
    aspect_ratio_x = scene.render.pixel_aspect_x
    aspect_ratio_y = scene.render.pixel_aspect_y
    fov_angle = bpy.data.cameras[0].angle
    camera_shift_x = bpy.data.cameras[0].shift_x
    camera_shift_y = bpy.data.cameras[0].shift_y

    fs.serialize('PerspectiveCameraEntity')
    fs.serialize(exporter_common.vec3_to_tuple(pos))
    fs.serialize(exporter_common.vec3_to_tuple(up))
    fs.serialize(exporter_common.vec3_to_tuple(target))
    fs.serialize(camera.data.sort_camera.sort_camera_lens.lens_size)
    fs.serialize((sensor_w,sensor_h))
    fs.serialize(int(sensor_fit))
    fs.serialize((aspect_ratio_x,aspect_ratio_y))
    fs.serialize(fov_angle)

    total_vert_cnt = 0
    total_normal_cnt = 0
    total_uv_cnt = 0
    total_face_cnt = 0
    for ob in exporter_common.getMeshList(scene):
        fs.serialize('VisualEntity')
        fs.serialize( exporter_common.matrix_to_tuple( MatrixBlenderToSort() * ob.matrix_world ) )
        stat = export_mesh(ob, scene, fs)
        total_vert_cnt += stat[0]
        total_normal_cnt += stat[1]
        total_uv_cnt += stat[2]
        total_face_cnt += stat[3]
    exporter_common.log( "Total vertices: %d." % total_vert_cnt )
    exporter_common.log( "Total normals: %d." % total_normal_cnt )
    exporter_common.log( "Total uvs: %d." % total_uv_cnt )
    exporter_common.log( "Total faces: %d." % total_face_cnt )

    for ob in exporter_common.getLightList(scene):
        lamp = ob.data
        # light faces forward Y+ in SORT, while it faces Z- in Blender, needs to flip the direction
        flip_mat = mathutils.Matrix([[ 1.0 , 0.0 , 0.0 , 0.0 ] , [ 0.0 , -1.0 , 0.0 , 0.0 ] , [ 0.0 , 0.0 , 1.0 , 0.0 ] , [ 0.0 , 0.0 , 0.0 , 1.0 ]])
        world_matrix = MatrixBlenderToSort() * ob.matrix_world * MatrixSortToBlender() * flip_mat

        if lamp.type == 'SUN':
            light_spectrum = np.array(lamp.color[:])
            light_spectrum *= lamp.energy

            fs.serialize('DirLightEntity')
            fs.serialize(exporter_common.matrix_to_tuple(world_matrix))
            fs.serialize(exporter_common.vec3_to_tuple(light_spectrum))
        elif lamp.type == 'POINT':
            light_spectrum = np.array(lamp.color[:])
            light_spectrum *= lamp.energy

            fs.serialize('PointLightEntity')
            fs.serialize(exporter_common.matrix_to_tuple(world_matrix))
            fs.serialize(exporter_common.vec3_to_tuple(light_spectrum))
        elif lamp.type == 'SPOT':
            light_spectrum = np.array(lamp.color[:])
            light_spectrum *= lamp.energy
            falloff_start = degrees(lamp.spot_size * ( 1.0 - lamp.spot_blend ) * 0.5)
            falloff_range = degrees(lamp.spot_size*0.5)

            fs.serialize('SpotLightEntity')
            fs.serialize(exporter_common.matrix_to_tuple(world_matrix))
            fs.serialize(exporter_common.vec3_to_tuple(light_spectrum))
            fs.serialize(falloff_start)
            fs.serialize(falloff_range)
        elif lamp.type == 'AREA':
            light_spectrum = np.array(lamp.color[:])
            light_spectrum *= lamp.energy
            sizeX = lamp.size
            sizeY = lamp.size_y
            if lamp.shape == 'SQUARE':
                sizeY = lamp.size

            fs.serialize('AreaLightEntity')
            fs.serialize(exporter_common.matrix_to_tuple(world_matrix))
            fs.serialize(exporter_common.vec3_to_tuple(light_spectrum))
            fs.serialize(sizeX)
            fs.serialize(sizeY)
        elif lamp.type == 'HEMI':
            light_spectrum = np.array(lamp.color[:])
            light_spectrum *= lamp.energy

            fs.serialize('SkyLightEntity')
            fs.serialize(exporter_common.matrix_to_tuple(MatrixBlenderToSort() * ob.matrix_world * MatrixSortToBlender()))
            fs.serialize(exporter_common.vec3_to_tuple(light_spectrum))
            fs.serialize(bpy.path.abspath( lamp.sort_lamp.sort_lamp_hemi.envmap_file ))

    # to indicate the scene stream comes to an end
    fs.serialize('')

mtl_dict = {}
mtl_rev_dict = {}

def name_compat(name):
    if name is None:
        return 'None'
    else:
        return name.replace(' ', '_')

def triangulate_object(obj):
    me = obj.data
    # Get a BMesh representation
    bm = bmesh.new()
    bm.from_mesh(me)

    bmesh.ops.triangulate(bm, faces=bm.faces[:], quad_method=0, ngon_method=0)

    # Finish up, write the bmesh back to the mesh
    bm.to_mesh(me)
    bm.free()

done = True
def test( obj ):
    global done
    if done:
        return
    done = True

    mesh = obj.data

    normals = []
    indices = []
    for face in mesh.polygons:
        indices.append(face.vertices[0])
        indices.append(face.vertices[1])
        indices.append(face.vertices[2])
        for i in range(len(face.vertices)):
            v = mesh.vertices[face.vertices[i]]
            normals.append([v.normal[0],v.normal[1],v.normal[2]])

    verts = []
    for vert in mesh.vertices:
        verts.append(vert.co.xyz)

    uvs = []
    for uv_layer in mesh.uv_layers:
        for x in range(len(uv_layer.data)):
            uvs.append(uv_layer.data[x].uv)

    print(indices)
    print(verts)
    print(uvs)
    print(normals)

# export mesh file
def export_mesh(obj,scene,fs):
    global done

    exporter_common.logD("Exporting object: %s." % obj.name)
    # make sure there is no quad in the object
    current_time = time.time()
    triangulate_object(obj)
    exporter_common.logD("Triangulating object. %.2f (s)" % (time.time() - current_time))
    current_time = time.time()

    # the mesh object
    mesh = obj.data

    faceuv = len(mesh.uv_textures) > 0
    if faceuv:
        uv_layer = mesh.uv_layers.active.data[:]

    # face index pairs
    face_index_pairs = [(face, index) for index, face in enumerate(mesh.polygons)]

    # generate normal data
    mesh.calc_normals_split()
    exporter_common.logD("Calculating split normals. %.2f(s), %d vertices." %((time.time() - current_time),len(mesh.vertices)))
    current_time = time.time()
    contextMat = None
    materials = mesh.materials[:]
    material_names = [m.name if m else None for m in materials]

    # avoid bad index errors
    if not materials:
        materials = [None]
        material_names = [name_compat(None)]

    name1 = obj.name
    name2 = obj.data.name
    obnamestring = '%s_%s' % (name_compat(name1), name_compat(name2))

    # serialize vertices
    fs.serialize(len(mesh.vertices))
    for v in mesh.vertices:
        fs.serialize(v.co[:])

        if done is False:
            print(v.co[:])

    vert_cnt = len(mesh.vertices)
    exporter_common.logD("Exporting vertices. %.2f(s), %d vertices." % ((time.time() - current_time) , vert_cnt))
    current_time = time.time()

    # UV
    uvs = []
    uv_unique_count = 0
    testw = {1}
    testw.add(1)
    if faceuv:
        # in case removing some of these dont get defined.
        uv = f_index = uv_index = uv_key = uv_val = uv_ls = None

        uv_face_mapping = [None] * len(face_index_pairs)
        uv_dict = {}
        uv_get = uv_dict.get
        for f, f_index in face_index_pairs:
            uv_ls = uv_face_mapping[f_index] = []
            for uv_index, l_index in enumerate(f.loop_indices):
                uv = uv_layer[l_index].uv
                #testw.add( l_index )
                uv_key = round(uv[0], 4), round(uv[1], 4)
                uv_val = uv_get(uv_key)
                if uv_val is None:
                    uv_val = uv_dict[uv_key] = uv_unique_count
                    
                    uvs.append( uv[:] )
                    uv_unique_count += 1
                uv_ls.append(uv_val)
        del uv_dict, uv, f_index, uv_index, uv_ls, uv_get, uv_key, uv_val

    # serialize uv coordinate
    #print( "%d %d" % ( len(uvs) , len( uvs ) ) )
    fs.serialize(len(uvs))
    for uv in uvs:
        if done is False:
            print(uv[:])
        fs.serialize(uv[:])
    uv_cnt = len(uvs)
    del uvs
    exporter_common.logD("Exporting uvs. %.2f(s), %d uvs." %((time.time() - current_time),uv_cnt))
    current_time = time.time()

    # output normal
    nor_key = nor_val = None
    normals_to_idx = {}
    nor_get = normals_to_idx.get
    nor_unique_count = 0
    loops_to_normals = [0] * len(mesh.loops)
    normals = []
    for f, f_index in face_index_pairs:
        for l_idx in f.loop_indices:
            def veckey3d(v):
                return round(v.x, 4), round(v.y, 4), round(v.z, 4)
            nor_key = veckey3d(mesh.loops[l_idx].normal)
            nor_val = nor_get(nor_key)
            if nor_val is None:
                nor_val = normals_to_idx[nor_key] = nor_unique_count
                normals.append( nor_key )
                nor_unique_count += 1
            loops_to_normals[l_idx] = nor_val
    del normals_to_idx, nor_get, nor_key, nor_val

    # serialize normals
    fs.serialize(len(normals))
    for normal in normals:
        if done is False:
            print(normal[:])
        fs.serialize(normal[:])
    normal_cnt = len(normals)
    del normals
    exporter_common.logD("Exporting normals. %.2f(s), %d normals." %((time.time() - current_time),normal_cnt))
    current_time = time.time()

    me_verts = mesh.vertices

    class Trunk:
        def __init__(self, mat_name):
            self.mat_name = mat_name
            self.face = []

    trunks = []

    for f, f_index in face_index_pairs:
        f_smooth = f.use_smooth
        f_mat = min(f.material_index, len(materials) - 1)

        key = material_names[f_mat], None  # No image, use None instead.

        # CHECK FOR CONTEXT SWITCH
        if key == contextMat:
            pass  # Context already switched, dont do anything
        else:
            if key[0] is None and key[1] is None:
                pass
            else:
                mat_data = mtl_dict.get(key)
                if not mat_data:
                    # First add to global dict so we can export to mtl
                    # Then write mtl

                    # Make a new names from the mat and image name,
                    # converting any spaces to underscores with name_compat.

                    # If none image dont bother adding it to the name
                    # Try to avoid as much as possible adding texname (or other things)
                    # to the mtl name (see [#32102])...
                    mtl_name = "%s" % name_compat(key[0])
                    if mtl_rev_dict.get(mtl_name, None) not in {key, None}:
                        if key[1] is None:
                            tmp_ext = "_NONE"
                        else:
                            tmp_ext = "_%s" % name_compat(key[1])
                        i = 0
                        while mtl_rev_dict.get(mtl_name + tmp_ext, None) not in {key, None}:
                            i += 1
                            tmp_ext = "_%3d" % i
                        mtl_name += tmp_ext
                    mat_data = mtl_dict[key] = mtl_name, materials[f_mat], None
                    mtl_rev_dict[mtl_name] = key

                trunks.append( Trunk(mat_data[0]) )

        # update current context material
        contextMat = key

        # output face information
        f_v = [(vi, me_verts[v_idx], l_idx)
                   for vi, (v_idx, l_idx) in enumerate(zip(f.vertices, f.loop_indices))]

        totverts = 1
        totuvco = 1
        totno = 1
        if faceuv:
            for vi, v, li in f_v:
                trunks[-1].face.append( (totverts + v.index, totuvco + uv_face_mapping[f_index][vi], totno + loops_to_normals[li] ) )
        else:  # No UV's
            for vi, v, li in f_v:
                trunks[-1].face.append( (totverts + v.index, totno + loops_to_normals[li]) )

    #serialize trunks
    face_cnt = 0
    fs.serialize( len(trunks) )
    for trunk in trunks:
        fs.serialize( trunk.mat_name )
        fs.serialize( int(len( trunk.face ) / 3) )
        face_cnt += len(trunk.face) / 3
        for ids in trunk.face:
            if done is False:
                print( ids[:] )
            for id in ids:
                fs.serialize( id - 1 )
    del trunks
    exporter_common.logD("Exporting faces. %.2f(s), %d faces." %((time.time() - current_time),face_cnt))
    current_time = time.time()

    test(obj)

    return ( vert_cnt , normal_cnt , uv_cnt , face_cnt )

def export_global_config(scene, fs, sort_resource_path):
    # global renderer configuration
    sort_output_file = 'blender_generated.exr'
    xres = scene.render.resolution_x * scene.render.resolution_percentage / 100
    yres = scene.render.resolution_y * scene.render.resolution_percentage / 100
    integrator_type = scene.integrator_type_prop

    fs.serialize( 0 )
    fs.serialize( sort_resource_path )
    fs.serialize( sort_output_file )
    fs.serialize( 64 )    # tile size, hard-coded it until I need to update it throught exposed interface later.
    fs.serialize( int(scene.thread_num_prop) )
    fs.serialize( int(scene.sampler_count_prop) )
    fs.serialize( int(xres) )
    fs.serialize( int(yres) )
    fs.serialize( scene.accelerator_type_prop )
    fs.serialize( scene.integrator_type_prop )
    fs.serialize( int(scene.inte_max_recur_depth) )
    if integrator_type == "ao":
        fs.serialize( scene.ao_max_dist )
    if integrator_type == "bdpt":
        fs.serialize( bool(scene.bdpt_mis) )
    if integrator_type == 'ir':
        fs.serialize( scene.ir_light_path_set_num )
        fs.serialize( scene.ir_light_path_num )
        fs.serialize( scene.ir_min_dist )

def export_material(scene, fs):
    # find the output node, duplicated code, to be cleaned
    def find_output_node(material):
        if material and material.sort_material and material.sort_material.sortnodetree:
            ntree = bpy.data.node_groups[material.sort_material.sortnodetree]
            for node in ntree.nodes:
                if getattr(node, "bl_idname", None) == 'SORTNodeOutput':
                    return node
        return None

    material_count = 0
    for material in exporter_common.getMaterialList(scene):
        # get output nodes
        output_node = find_output_node(material)
        if output_node is None:
            continue
        material_count += 1

    fs.serialize( int(material_count) )
    for material in exporter_common.getMaterialList(scene):
        # get the sort tree nodes
        ntree = bpy.data.node_groups[material.sort_material.sortnodetree]

        # get output nodes
        output_node = find_output_node(material)
        if output_node is None:
            continue

        fs.serialize( name_compat(material.name) )

        def serialize_prop(mat_node , fs):
            # output the properties
            seriliaze_prop_in_sort = lambda n , v , fs = fs : ( fs.serialize( '' ) , fs.serialize( v ) )
            mat_node.serializae_prop(seriliaze_prop_in_sort)

            inputs = mat_node.inputs
            for socket in inputs:
                if socket.is_linked:
                    def socket_node_input(nt, socket):
                        return next((l.from_node for l in nt.links if l.to_socket == socket), None)
                    input_node = socket_node_input(ntree, socket)

                    fs.serialize(input_node.bl_idname)
                    serialize_prop(input_node,fs)
                else:
                    fs.serialize('')
                    fs.serialize( socket.export_serialization_value() )
        serialize_prop(output_node, fs)
    del fs

