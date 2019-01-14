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
import os
import mathutils
import shutil
import numpy as np
import platform
import tempfile
import bmesh
import time
import struct
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
    exporter_common.log("Exported materials %.2f(s)" % (time.time() - current_time))
    current_time = time.time()

    # export scene
    exporter_common.log("Exporting scene.")
    export_scene(scene, fs)
    exporter_common.log("Exported scene %.2f(s)" % (time.time() - current_time))
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
        stat = export_mesh(ob.data,fs)
        total_vert_cnt += stat[0]
        total_face_cnt += stat[1]
    exporter_common.log( "Total vertices: %d." % total_vert_cnt )
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

# avoid having space in material name
def name_compat(name):
    if name is None:
        return 'None'
    else:
        return name.replace(' ', '_')

def export_mesh(mesh, fs):
    LENFMT = struct.Struct('=i')
    FLTFMT = struct.Struct('=f')
    VERTFMT = struct.Struct('=ffffffff')
    LINEFMT = struct.Struct('=iiffi')
    POINTFMT = struct.Struct('=fff')
    TRIFMT = struct.Struct('=iiii')

    materials = mesh.materials[:]
    material_names = [m.name if m else None for m in materials]

    verti = 0
    trii = 0
    verts = mesh.vertices
    wo3_verts = bytearray()

    if len(mesh.polygons) is 0 :
        # hacking parameters for now
        bottom = 0.2
        tip = 0.0
        hair_step = 5
        hair_step_inv = 0.2
        i = 0

        # taking the first material for now.
        global matname_to_id
        matid = -1
        matname = name_compat(mesh.materials[0].name) if len( mesh.materials ) > 0 else None
        matid = matname_to_id[matname] if matname in matname_to_id else -1

        line_verts = bytearray()
        for vert in verts:
            wo3_verts += POINTFMT.pack( vert.co[0] , vert.co[1] , vert.co[2] )

        for edge in mesh.edges:
            v0 = edge.vertices[0]
            v1 = edge.vertices[1]
            t = ( hair_step - i ) * hair_step_inv
            w0 = t * bottom + ( 1.0 - t ) * tip
            w1 = w0 + ( tip - bottom ) * hair_step_inv
            line_verts += LINEFMT.pack(v0,v1,w0,w1,matid)
            i = ( i + 1 ) % 5

        fs.serialize( 'LineSetVisual' )
        fs.serialize( LENFMT.pack( len(verts) ) )
        fs.serialize( wo3_verts )
        fs.serialize( LENFMT.pack( len(mesh.edges) ) )
        fs.serialize( line_verts )
    else:
        # output the mesh information.
        mesh.calc_normals()
        if not mesh.tessfaces and mesh.polygons:
            mesh.calc_tessface()

        has_uv = bool(mesh.tessface_uv_textures)

        if has_uv:
            active_uv_layer = mesh.tessface_uv_textures.active
            if not active_uv_layer:
                has_uv = False
            else:
                active_uv_layer = active_uv_layer.data

        wo3_indices = [{} for _ in range(len(verts))]
        wo3_tris = bytearray()

        uvcoord = (0.0, 0.0)
        for i, f in enumerate(mesh.tessfaces):
            smooth = f.use_smooth
            if not smooth:
                normal = f.normal[:]

            if has_uv:
                uv = active_uv_layer[i]
                uv = (uv.uv1, uv.uv2, uv.uv3, uv.uv4)

            oi = []
            for j, vidx in enumerate(f.vertices):
                v = verts[vidx]

                if smooth:
                    normal = v.normal[:]

                if has_uv:
                    uvcoord = (uv[j][0], uv[j][1])

                key = (normal, uvcoord)
                out_idx = wo3_indices[vidx].get(key)
                if out_idx is None:
                    out_idx = verti
                    wo3_indices[vidx][key] = out_idx
                    wo3_verts += VERTFMT.pack(v.co[0], v.co[1], v.co[2], normal[0], normal[1], normal[2], uvcoord[0], uvcoord[1])
                    verti += 1

                oi.append(out_idx)

            global matname_to_id
            matid = -1
            matname = name_compat(material_names[f.material_index]) if len( material_names ) > 0 else None
            matid = matname_to_id[matname] if matname in matname_to_id else -1
            if len(oi) == 3:
                # triangle
                wo3_tris += TRIFMT.pack(oi[0], oi[1], oi[2], matid)
                trii += 1
            else:
                # quad
                wo3_tris += TRIFMT.pack(oi[0], oi[1], oi[2], matid)
                wo3_tris += TRIFMT.pack(oi[0], oi[2], oi[3], matid)
                trii += 2

        fs.serialize('MeshVisual')
        fs.serialize(bool(has_uv))
        fs.serialize(LENFMT.pack(verti))
        fs.serialize(wo3_verts)
        fs.serialize(LENFMT.pack(trii))
        fs.serialize(wo3_tris)

    return (verti, trii)

def export_global_config(scene, fs, sort_resource_path):
    # global renderer configuration
    sort_output_file = 'blender_generated.exr'
    xres = scene.render.resolution_x * scene.render.resolution_percentage / 100
    yres = scene.render.resolution_y * scene.render.resolution_percentage / 100
    integrator_type = scene.integrator_type_prop
    accelerator_type = scene.accelerator_type_prop

    fs.serialize( 0 )
    fs.serialize( sort_resource_path )
    fs.serialize( sort_output_file )
    fs.serialize( 64 )    # tile size, hard-coded it until I need to update it throught exposed interface later.
    fs.serialize( int(scene.thread_num_prop) )
    fs.serialize( int(scene.sampler_count_prop) )
    fs.serialize( int(xres) )
    fs.serialize( int(yres) )
    fs.serialize( accelerator_type )
    if accelerator_type == "bvh":
        fs.serialize( int(scene.bvh_max_node_depth) )
        fs.serialize( int(scene.bvh_max_pri_in_leaf) )
    elif accelerator_type == "KDTree":
        fs.serialize( int(scene.kdtree_max_node_depth) )
        fs.serialize( int(scene.kdtree_max_pri_in_leaf) )
    elif accelerator_type == "OcTree":
        fs.serialize( int(scene.octree_max_node_depth) )
        fs.serialize( int(scene.octree_max_pri_in_leaf) )

    fs.serialize( integrator_type )
    fs.serialize( int(scene.inte_max_recur_depth) )
    if integrator_type == "AmbientOcclusion":
        fs.serialize( scene.ao_max_dist )
    if integrator_type == "BidirPathTracing" or integrator_type == "LightTracing":
        fs.serialize( bool(scene.bdpt_mis) )
    if integrator_type == "InstantRadiosity":
        fs.serialize( scene.ir_light_path_set_num )
        fs.serialize( scene.ir_light_path_num )
        fs.serialize( scene.ir_min_dist )

matname_to_id = {}
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

    global matname_to_id
    i = 0
    fs.serialize( int(material_count) )
    for material in exporter_common.getMaterialList(scene):
        # get the sort tree nodes
        ntree = bpy.data.node_groups[material.sort_material.sortnodetree]

        # get output nodes
        output_node = find_output_node(material)
        if output_node is None:
            continue

        matname_to_id[name_compat(material.name)] = i
        i += 1
        fs.serialize( name_compat(material.name) )
        exporter_common.logD( 'Exporting material %s.' % name_compat(material.name) )

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

                    fs.serialize(input_node.sort_bxdf_type)
                    serialize_prop(input_node,fs)
                else:
                    fs.serialize('')
                    fs.serialize( socket.export_serialization_value() )
        serialize_prop(output_node, fs)
    del fs

