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
import mathutils
import numpy as np
import platform
import tempfile
import time
import struct
from math import degrees
from . import export_common
from ..stream import stream

def get_sort_dir():
    return_path = export_common.getPreference().install_path
    if platform.system() == 'Windows':
        return return_path
    return return_path

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
    if platform.system() == 'Windows':
        return_path = return_path.replace( '\\' , '/' )
        return return_path + '/'
    return return_path + '/'

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
    ori_matrix = MatrixBlenderToSort() @ camera.matrix_world.copy()
    # get the transpose matrix
    matrix = ori_matrix.transposed()
    pos = matrix[3]             # get eye position
    forwards = -matrix[2]       # get forward direction

    # get focal distance for DOF effect
    """
    if camera.data.dof_object is not None:
        focal_object = camera.data.dof_object
        fo_mat = MatrixBlenderToSort() @ focal_object.matrix_world
        delta = fo_mat.to_translation() - pos.to_3d()
        focal_distance = delta.dot(forwards)
    else:
        focal_distance = max( camera.data.dof_distance , 0.01 )
    """

    focal_distance = 0.01
    scaled_forward = mathutils.Vector((focal_distance * forwards[0], focal_distance * forwards[1], focal_distance * forwards[2] , 0.0))
    # viewing target
    target = (pos + scaled_forward)
    # up direction
    up = matrix[1]
    return (pos, target, up)

# export blender information
def export_blender(depsgraph, force_debug=False):
    scene = depsgraph.scene
    export_common.setScene(scene)

    # create immediate file path
    sort_resource_path = create_path(scene, force_debug)
    sort_config_file = sort_resource_path + 'scene.sort'
    fs = stream.FileStream( sort_config_file )

    export_common.log("Exporting sort file %s" % sort_config_file)

    # export global material settings
    current_time = time.time()
    export_common.log("Exporting global configuration.")
    export_global_config(scene, fs, sort_resource_path)
    export_common.log("Exported configuration %.2f" % (time.time() - current_time))
    current_time = time.time()

    # export material
    export_common.log("Exporting materials.")
    collect_shader_resources(scene, fs)
    export_materials(scene, fs)
    export_common.log("Exported materials %.2f(s)" % (time.time() - current_time))
    current_time = time.time()

    # export scene
    export_common.log("Exporting scene.")
    export_scene(depsgraph, fs)
    export_common.log("Exported scene %.2f(s)" % (time.time() - current_time))
    current_time = time.time()

    fs.flush()
    del fs

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
def export_scene(depsgraph, fs):
    scene = depsgraph.scene
    fs.serialize( int(1234567) )

    # camera node
    camera = export_common.getCamera(scene)
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
    fs.serialize(export_common.vec3_to_tuple(pos))
    fs.serialize(export_common.vec3_to_tuple(up))
    fs.serialize(export_common.vec3_to_tuple(target))
    fs.serialize( 0.0 )
    #fs.serialize(camera.data.sort_camera.sort_camera_lens.lens_size)
    fs.serialize((sensor_w,sensor_h))
    fs.serialize(int(sensor_fit))
    fs.serialize((aspect_ratio_x,aspect_ratio_y))
    fs.serialize(fov_angle)

    total_vert_cnt = 0
    total_prim_cnt = 0
    for obj in export_common.getMeshList(scene):
        fs.serialize('VisualEntity')
        fs.serialize( export_common.matrix_to_tuple( MatrixBlenderToSort() @ obj.matrix_world ) )
        fs.serialize( 1 )   # only one mesh for each mesh entity
        stat = None
        # apply the modifier if there is one
        if obj.type != 'MESH' or obj.is_modified(scene, 'RENDER'):
            try:
                # create a temporary mesh
                mesh = obj.to_mesh(depsgraph , True)
                # instead of exporting the original mesh, export the temporary mesh.
                stat = export_mesh(mesh, fs)
            finally:
                # delete the temporay mesh
                bpy.data.meshes.remove(mesh)
        else:
            stat = export_mesh(obj.data, fs)

        total_vert_cnt += stat[0]
        total_prim_cnt += stat[1]

    for obj in export_common.getMeshList(scene):
        # output hair/fur information
        if len( obj.particle_systems ) > 0:
            fs.serialize('VisualEntity')
            fs.serialize( export_common.matrix_to_tuple( MatrixBlenderToSort() @ obj.matrix_world ) )
            fs.serialize( len( obj.particle_systems ) )
            for ps in obj.particle_systems:
                stat = export_hair( ps , obj , scene , fs )
                total_vert_cnt += stat[0]
                total_prim_cnt += stat[1]
    export_common.log( "Total vertices: %d." % total_vert_cnt )
    export_common.log( "Total primitives: %d." % total_prim_cnt )

    for ob in export_common.getLightList(scene):
        lamp = ob.data
        # light faces forward Y+ in SORT, while it faces Z- in Blender, needs to flip the direction
        flip_mat = mathutils.Matrix([[ 1.0 , 0.0 , 0.0 , 0.0 ] , [ 0.0 , -1.0 , 0.0 , 0.0 ] , [ 0.0 , 0.0 , 1.0 , 0.0 ] , [ 0.0 , 0.0 , 0.0 , 1.0 ]])
        world_matrix = MatrixBlenderToSort() @ ob.matrix_world @ MatrixSortToBlender() @ flip_mat

        if lamp.type == 'SUN':
            light_spectrum = np.array(lamp.color[:])
            light_spectrum *= lamp.energy

            fs.serialize('DirLightEntity')
            fs.serialize(export_common.matrix_to_tuple(world_matrix))
            fs.serialize(export_common.vec3_to_tuple(light_spectrum))
        elif lamp.type == 'POINT':
            light_spectrum = np.array(lamp.color[:])
            light_spectrum *= lamp.energy

            fs.serialize('PointLightEntity')
            fs.serialize(export_common.matrix_to_tuple(world_matrix))
            fs.serialize(export_common.vec3_to_tuple(light_spectrum))
        elif lamp.type == 'SPOT':
            light_spectrum = np.array(lamp.color[:])
            light_spectrum *= lamp.energy
            falloff_start = degrees(lamp.spot_size * ( 1.0 - lamp.spot_blend ) * 0.5)
            falloff_range = degrees(lamp.spot_size*0.5)

            fs.serialize('SpotLightEntity')
            fs.serialize(export_common.matrix_to_tuple(world_matrix))
            fs.serialize(export_common.vec3_to_tuple(light_spectrum))
            fs.serialize(falloff_start)
            fs.serialize(falloff_range)
        elif lamp.type == 'AREA':
            light_spectrum = np.array(lamp.color[:])
            light_spectrum *= lamp.energy
            sizeX = lamp.size
            sizeY = lamp.size_y

            fs.serialize('AreaLightEntity')
            fs.serialize(export_common.matrix_to_tuple(world_matrix))
            fs.serialize(export_common.vec3_to_tuple(light_spectrum))

            fs.serialize( lamp.shape )
            if lamp.shape == 'SQUARE':
                fs.serialize(sizeX)
            elif lamp.shape == 'RECTANGLE':
                fs.serialize(sizeX)
                fs.serialize(sizeY)
            elif lamp.shape == 'DISK':
                fs.serialize(sizeX * 0.5)
            
    hdr_sky_image = scene.sort_hdr_sky.hdr_image
    if hdr_sky_image is not None:
        fs.serialize('SkyLightEntity')
        fs.serialize(export_common.matrix_to_tuple(MatrixBlenderToSort() @ MatrixSortToBlender()))
        fs.serialize(( 1.0 , 1.0 , 1.0 ))
        fs.serialize(bpy.path.abspath( hdr_sky_image.filepath ))

    # to indicate the scene stream comes to an end
    fs.serialize('')

# avoid having space in material name
def name_compat(name):
    if name is None:
        return 'None'
    else:
        return name.replace(' ', '_')

def export_hair(ps, obj, scene, fs):
    LENFMT = struct.Struct('=i')
    POINTFMT = struct.Struct('=fff')

    #ps.set_resolution(scene, obj, 'RENDER')

    hairs = ps.particles
    
    vert_cnt = 0
    render_step = ps.settings.render_step
    width_tip = ps.settings.sort_hair.hair_tip
    width_bottom = ps.settings.sort_hair.hair_bottom

    # extract the material of the hair
    mat_local_index = ps.settings.material
    mat_index = -1

    if mat_local_index > 0 and mat_local_index <= len( obj.data.materials ):
        mat_name = name_compat(obj.data.materials[mat_local_index-1].name)
        mat_index = matname_to_id[mat_name] if mat_name in matname_to_id else -1

    # for some unknown reason
    steps = 2 ** render_step

    verts = bytearray()

    world2Local = obj.matrix_world.inverted()
    num_parents = len(ps.particles)
    num_children = len(ps.child_particles)
    hair_cnt = num_parents + num_children
    total_hair_segs = 0
    for pindex in range(hair_cnt):
        hair = []
        for step in range(0, steps + 1):
            co = ps.co_hair(obj, particle_no = pindex, step = step)
            # there could be a bug of ignoring point at origin
            if not co.length_squared == 0:
                co = world2Local @ co
                hair.append( co )
                vert_cnt += 1
            else:
                break

        assert len(hair) > 0
        verts += LENFMT.pack( len(hair) - 1 )
        for h in hair :
            verts += POINTFMT.pack( h[0] , h[1] , h[2] )
        total_hair_segs += len(hair) - 1

    #ps.set_resolution(scene, obj, 'PREVIEW')

    fs.serialize( 'HairVisual' )
    fs.serialize( hair_cnt )
    fs.serialize( width_tip )
    fs.serialize( width_bottom )
    fs.serialize( mat_index )
    fs.serialize( verts )

    return (vert_cnt, total_hair_segs)

# warning, this export function is not an optimal version, but it works.
# I will get back to it some time later.
def export_mesh(mesh, fs):
    LENFMT = struct.Struct('=i')
    FLTFMT = struct.Struct('=f')
    VERTFMT = struct.Struct('=ffffffff')
    LINEFMT = struct.Struct('=iiffi')
    POINTFMT = struct.Struct('=fff')
    TRIFMT = struct.Struct('=iiii')

    materials = mesh.materials[:]
    material_names = [m.name if m else None for m in materials]

    vert_cnt = 0
    primitive_cnt = 0
    verts = mesh.vertices
    wo3_verts = bytearray()

    global matname_to_id

    # output the mesh information.
    mesh.calc_normals()
    mesh.calc_loop_triangles()

    has_uv = bool(mesh.uv_layers)
    uv_layer = None
    if has_uv:
        active_uv_layer = mesh.uv_layers.active
        if not active_uv_layer:
            has_uv = False
        else:
            uv_layer = active_uv_layer.data

    wo3_indices = [{} for _ in range(len(verts))]
    wo3_tris = bytearray()

    uvs = [[0.0,0.0]] * len( verts )
    if has_uv:
        for poly in mesh.polygons:
            for loop_index in range(poly.loop_start, poly.loop_start + poly.loop_total):
                vid = mesh.loops[loop_index].vertex_index
                uvs[vid] = [uv_layer[loop_index].uv.x , uv_layer[loop_index].uv.y]

    uvcoord = (0.0, 0.0)
    for i, f in enumerate(mesh.loop_triangles):
        smooth = f.use_smooth
        if not smooth:
            normal = f.normal[:]

        oi = []
        for j, vidx in enumerate(f.vertices):
            v = verts[vidx]

            if smooth:
                normal = v.normal[:]

            if has_uv:
                uv = uvs[vidx]
                uvcoord = ( uv[0], uv[1] )

            key = (normal, uvcoord)
            out_idx = wo3_indices[vidx].get(key)
            if out_idx is None:
                out_idx = vert_cnt
                wo3_indices[vidx][key] = out_idx
                wo3_verts += VERTFMT.pack(v.co[0], v.co[1], v.co[2], normal[0], normal[1], normal[2], uvcoord[0], uvcoord[1])
                vert_cnt += 1

            oi.append(out_idx)

        matid = -1
        matname = name_compat(material_names[f.material_index]) if len( material_names ) > 0 else None
        matid = matname_to_id[matname] if matname in matname_to_id else -1
        if len(oi) == 3:
            # triangle
            wo3_tris += TRIFMT.pack(oi[0], oi[1], oi[2], matid)
            primitive_cnt += 1
        else:
            assert( len(oi) == 4 )
            # quad
            wo3_tris += TRIFMT.pack(oi[0], oi[1], oi[2], matid)
            wo3_tris += TRIFMT.pack(oi[0], oi[2], oi[3], matid)
            primitive_cnt += 2

    fs.serialize('MeshVisual')
    fs.serialize(bool(has_uv))
    fs.serialize(LENFMT.pack(vert_cnt))
    fs.serialize(wo3_verts)
    fs.serialize(LENFMT.pack(primitive_cnt))
    fs.serialize(wo3_tris)

    return (vert_cnt, primitive_cnt)

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

# find the output node, duplicated code, to be cleaned
def find_output_node(material):
    if material and material.sort_material:
        ntree = material.sort_material
        for node in ntree.nodes:
            if getattr(node, "bl_idname", None) == 'SORTNodeOutput':
                return node
    return None

# get the from node of this socket if there is one recursively
def get_from_socket(socket, parent_node_stack, visited):
    if not socket.is_linked:
        return None
    other = socket.links[0].from_socket

    if other.node in visited:
        return None

    if other.node.bl_idname == 'NodeReroute':
        return get_from_socket(other.node.inputs[0], parent_node_stack, visited )
    else:
        return other

# This function will iterate through all visited nodes in the scene and populate everything in a hash table
# Apart from collecting shaders, it will also collect all heavy data, like measured BRDF data, texture.
def collect_shader_resources(scene, fs):
    # don't output any osl_shaders if using default materials
    if scene.allUseDefaultMaterial is True:
        fs.serialize( 0 )
        fs.serialize( 0 )
        return None

    osl_shaders = {}
    resources = []

    dummy = set()
    for material in export_common.getMaterialList(scene):
        # get output nodes
        output_node = find_output_node(material)
        if output_node is None:
            continue

        def serialize_prop(mat_node , shaders , parent_node_stack , leaving_group = False ):
            if mat_node is None:
                return

            cloned_parent_node_stack = parent_node_stack.copy()
            if mat_node.isGroupNode() and leaving_group is False:
                cloned_parent_node_stack.append( mat_node )
                output_node = mat_node.getGroupTree().nodes.get("Group Outputs")
                serialize_prop( output_node , shaders , cloned_parent_node_stack )
            else:
                for socket in mat_node.inputs:
                    from_socket = get_from_socket( socket , parent_node_stack , dummy )
                    if from_socket is not None:
                        serialize_prop( from_socket.node , shaders , parent_node_stack )

            if mat_node.isGroupInputNode():
                parent_node = cloned_parent_node_stack.pop()
                serialize_prop( parent_node , shaders , cloned_parent_node_stack , True )

            # populate the resources first
            mat_node.populateResources( resources )

            # populate the shader source code if it is not exported before
            if mat_node.isGroupInputNode():
                # may there is a real stack instead of list?
                parent_node = parent_node_stack.pop()
                parent_node_stack.append( parent_node )

                shader_type = parent_node.type_identifier() + '_GI'
                if shader_type not in shaders:
                    shaders[shader_type] = parent_node.generate_osl_source()
            elif mat_node.isGroupNode() is False:
                shader_type = mat_node.type_identifier()
                if mat_node.isGroupOutputNode():
                    parent_node = parent_node_stack.pop()
                    parent_node_stack.append( parent_node )
                    shader_type =  parent_node.type_identifier() + '_GO'
                if shader_type not in shaders:
                    shaders[shader_type] = mat_node.generate_osl_source()

        parent_node_stack = [None]
        serialize_prop(output_node, osl_shaders, parent_node_stack, False)

    fs.serialize( len( osl_shaders ) )
    for key , value in osl_shaders.items():
        fs.serialize( key )
        fs.serialize( value )
        export_common.logD( 'Exporting node source code for node %s. Source code: %s' %(key , value) )
    del osl_shaders
    fs.serialize( len( resources ) )
    for resource in resources:
        fs.serialize( resource[0] ) # type
        fs.serialize( resource[1] ) # external file name

# Export OSL shader group
matname_to_id = {}
def export_materials(scene, fs):
    materials = export_common.getMaterialList(scene)
    material_count = 0
    for material in materials:
        # get output nodes
        output_node = find_output_node(material)
        if output_node is None:
            continue
        material_count += 1

    if scene.allUseDefaultMaterial is True:
        fs.serialize( int(0) )
        return None

    global matname_to_id
    i = 0
    fs.serialize( int(material_count) )
    for material in materials:
        # get output nodes
        output_node = find_output_node(material)
        if output_node is None:
            continue

        compact_material_name = name_compat(material.name)
        matname_to_id[compact_material_name] = i
        i += 1
        fs.serialize( compact_material_name )
        export_common.logD( 'Exporting material %s.' % compact_material_name )

        # collect node count
        mat_nodes = []          # resulting nodes
        mat_connections = []    # connections between nodes
        visited = set()         # prevent a node to be serialized twice
        def collect_node_count(mat_node, visited, parent_node_stack, leaving_group = False):
            parent_node , accumulative_name = parent_node_stack.pop()
            parent_node_stack.append( ( parent_node , accumulative_name ) )

            cloned_parent_node_stack = parent_node_stack.copy()

            if mat_node.isGroupNode() and leaving_group is False:
                cloned_parent_node_stack.append( ( mat_node , accumulative_name + mat_node.name ) )
                output_node = mat_node.getGroupTree().nodes.get("Group Outputs")
                collect_node_count( output_node , visited , cloned_parent_node_stack )
            else:
                for socket in mat_node.inputs:
                    input_socket = get_from_socket( socket , parent_node_stack , visited )
                    if input_socket is None:
                        continue
                    input_node = input_socket.node

                    source_param = input_node.getShaderOutputParameterName(input_socket.name)
                    target_param = mat_node.getShaderInputParameterName(socket.name)

                    input_shader_name = input_node.getUniqueName() + accumulative_name
                    if input_node.isGroupInputNode():
                        input_shader_name = parent_node.getUniqueName() + '_GI' + accumulative_name
                    elif input_node.isGroupNode():
                        input_shader_name = input_node.getUniqueName() + '_GO' + accumulative_name + input_node.name

                    output_shader_name = mat_node.getUniqueName() + accumulative_name
                    if mat_node.isGroupNode():
                        output_shader_name = mat_node.getUniqueName() + '_GI' + accumulative_name + mat_node.name
                    elif mat_node.isGroupOutputNode():
                        output_shader_name = parent_node.getUniqueName() + '_GO' + accumulative_name

                    mat_connections.append( ( input_shader_name , source_param , output_shader_name, target_param ) )

                    collect_node_count(input_node, visited, cloned_parent_node_stack)

            if mat_node.isGroupInputNode():
                parent_node , _ = cloned_parent_node_stack.pop()
                collect_node_count( parent_node , visited , cloned_parent_node_stack , True )

            shader_name = ''
            shader_type = ''
            shader_node = None
            if mat_node.isGroupInputNode():
                shader_name = parent_node.getUniqueName() + '_GI' + accumulative_name
                shader_type = parent_node.type_identifier() + '_GI'
                shader_node = parent_node
            elif mat_node.isGroupOutputNode():
                shader_name = parent_node.getUniqueName() + '_GO' + accumulative_name
                shader_type = parent_node.type_identifier() + '_GO'
                shader_node = parent_node
            elif mat_node.isGroupNode() is False:
                shader_name = mat_node.getUniqueName() + accumulative_name
                shader_type = mat_node.type_identifier()
                shader_node = mat_node

            if shader_name not in visited and shader_name != '':
                fs.serialize( shader_name )
                fs.serialize( shader_type )
                shader_node.serialize_prop( fs )
                visited.add( shader_name )

        # collect all material nodes
        parent_node_stack = [ ( None , compact_material_name ) ]
        collect_node_count(output_node, visited, parent_node_stack)

        fs.serialize( '' )
        fs.serialize( 'verification_string' )

        # serialize this material
        fs.serialize( len( mat_connections ) )
        for connection in mat_connections:
            fs.serialize( connection[0] )
            fs.serialize( connection[1] )
            fs.serialize( connection[2] )
            fs.serialize( connection[3] )

    export_common.log( 'Exported %d materials in total.' %(len(materials)) )
