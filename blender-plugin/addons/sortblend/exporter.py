#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2023 by Jiayin Cao - All rights reserved.
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
import platform
import tempfile
import struct
import numpy as np
from time import time
from math import degrees
from .log import log, logD
from .strid import SID
from .stream import stream

BLENDER_VERSION = f'{bpy.app.version[0]}.{bpy.app.version[1]}'

def depsgraph_objects(depsgraph: bpy.types.Depsgraph):
    """ Iterates evaluated objects in depsgraph with ITERATED_OBJECT_TYPES """
    ITERATED_OBJECT_TYPES = ('MESH', 'LIGHT')
    for obj in depsgraph.objects:
        if obj.type in ITERATED_OBJECT_TYPES:
            yield obj.evaluated_get(depsgraph)

# Get the list of material for the whole scene, this function will only list materials that are currently
# attached to an object in the scene. Non-used materials will not be needed to be exported to SORT.
def list_materials( depsgraph ):
    exported_materials = []
    for ob in depsgraph_objects(depsgraph):
        if ob.type == 'MESH':
            for material in ob.data.materials[:]:
                # make sure it is a SORT material
                if material and material.sort_material:
                    # skip if the material is already exported
                    if exported_materials.count( material ) != 0:
                        continue
                    exported_materials.append( material )
    return exported_materials

def get_sort_dir():
    preferences = bpy.context.preferences.addons['sortblend'].preferences
    return_path = preferences.install_path
    if platform.system() == 'Windows':
        return return_path
    
    return_path = os.path.expanduser(return_path)
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
    return_path = intermediate_dir if force_debug is False else get_sort_dir()
    if platform.system() == 'Windows':
        return_path = return_path.replace( '\\' , '/' )
        return return_path + '/'
    if return_path[-1] == '/':
        return return_path
    return return_path + '/'

# Coordinate transformation
# Basically, the coordinate system of Blender and SORT is very different.
# In Blender, the coordinate system is as below and this is a right handed system
#
#        Z
#        ^   Y
#        |  /
#        | /
#        |/
#        ---------> X
#
# In SORT, the coordinate system is as below and this is a left handed system
#
#
#        Y
#        ^   Z
#        |  /
#        | /
#        |/
#        ----------> X
#

# There are few ways to convert data from SORT coordinate system to Blender's
# The following method adopts this way
#    - Flip the z axis first to make sure it is a right handed system as Blender's
#    - Swap Y and the new flipped Z channel. So the new Y channel will be the -Z in the flipped coordinate,
#      the new Z channel will be the flipped Y channel.
#
# So the final matrix is
#           | 1.0  0.0  0.0  0.0 |
#  M_flip = | 0.0  1.0  0.0  0.0 |
#           | 0.0  0.0 -1.0  0.0 |
#           | 0.0  0.0  0.0  1.0 |
#
#           | 1.0  0.0  0.0  0.0 |
#  M_swap = | 0.0  0.0 -1.0  0.0 |
#           | 0.0  1.0  0.0  0.0 |
#           | 0.0  0.0  0.0  1.0 |
#                         
#                         | 1.0  0.0  0.0  0.0 |
#  M = M_swap * M_flip =  | 0.0  0.0  1.0  0.0 |
#                         | 0.0  1.0  0.0  0.0 |
#                         | 0.0  0.0  0.0  1.0 |

def MatrixSortToBlender():
    global_matrix = mathutils.Matrix()
    global_matrix[1][1] = 0.0
    global_matrix[2][2] = 0.0
    global_matrix[2][1] = 1.0
    global_matrix[1][2] = 1.0
    return global_matrix

# Similarly, in order to convert data from Blender coordinate to SORT coodinate.
# Instead of deriving it like above, a simple way to do this is simply to take the inverse of the above matrix,
# which is exactly the same with the matrix itself.
def MatrixBlenderToSort():
    global_matrix = mathutils.Matrix()
    global_matrix[1][1] = 0.0
    global_matrix[2][2] = 0.0
    global_matrix[2][1] = 1.0
    global_matrix[1][2] = 1.0
    return global_matrix

# get camera data
def lookat_camera(camera):
    # it seems that the matrix return here is the inverse of view matrix.
    ori_matrix = MatrixBlenderToSort() @ camera.matrix_world.copy()
    # get the transpose matrix
    matrix = ori_matrix.transposed()
    pos = matrix[3]             # get eye position
    forwards = -matrix[2]       # get forward direction

    # get focal distance for DOF effect
    if camera.data.dof.focus_object is not None:
        focal_object = camera.data.dof.focus_object
        fo_mat = MatrixBlenderToSort() @ focal_object.matrix_world
        delta = fo_mat.to_translation() - pos.to_3d()
        focal_distance = delta.dot(forwards)
    else:
        focal_distance = max( camera.data.dof.focus_distance , 0.01 )

    scaled_forward = mathutils.Vector((focal_distance * forwards[0], focal_distance * forwards[1], focal_distance * forwards[2] , 0.0))
    target = (pos + scaled_forward)  # viewing target
    up = matrix[1]                   # up direction
    return (pos, target, up)

# export blender information
def export_blender(depsgraph, force_debug=False, is_preview=False):
    scene = depsgraph.scene

    # create intermediate resource path
    sort_resource_path = create_path(scene, force_debug)

    # initialize the file to be fed as main input for the renderer
    # this will potentially be replaced with socket streaming in the future
    sort_config_file = sort_resource_path + 'scene.sort'
    fs = stream.FileStream( sort_config_file )
    log("Exporting sort file %s" % sort_config_file)

    # export global settings for the renderer
    current_time = time()
    log("Exporting global configuration.")
    export_global_config(scene, fs, sort_resource_path)
    log("Exported configuration %.2f" % (time() - current_time))

    # export materials
    current_time = time()
    log("Exporting materials.")
    collect_shader_resources(depsgraph, scene, fs)     # this is the place for material to signal heavy resources, like textures, measured BRDF, etc.
    export_materials(depsgraph, fs)                    # this is the place for serializing OSL shader source code with proper default values.
    log("Exported materials %.2f(s)" % (time() - current_time))

    # export scene
    current_time = time()
    log("Exporting scene.")
    export_scene(depsgraph, is_preview, fs)
    log("Exported scene %.2f(s)" % (time() - current_time))

    # make sure the result of the file writting is flushed because it could be problematic on some machines
    fs.flush()
    del fs

# clear old data and create new path
def create_path(scene, force_debug):
    global intermediate_dir
    # only create the temporary folder when it is not exporting the scene
    if force_debug is False:
        intermediate_dir = tempfile.mkdtemp(suffix='w')
    # get immediate directory
    output_dir = get_intermediate_dir(force_debug)

    # create one if there is no such directory
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)
    return output_dir

def get_smoke_modifier(obj: bpy.types.Object):
    if BLENDER_VERSION >= '2.82':
        return next((modifier for modifier in obj.modifiers if modifier.type == 'FLUID' and modifier.fluid_type == 'DOMAIN'), None)
    return next((modifier for modifier in obj.modifiers if modifier.type == 'SMOKE' and modifier.smoke_type == 'DOMAIN'), None)

# export scene
def export_scene(depsgraph, is_preview, fs):
    # helper function to convert a matrix to a tuple
    def matrix_to_tuple(matrix):
        return (matrix[0][0],matrix[0][1],matrix[0][2],matrix[0][3],matrix[1][0],matrix[1][1],matrix[1][2],matrix[1][3],
                matrix[2][0],matrix[2][1],matrix[2][2],matrix[2][3],matrix[3][0],matrix[3][1],matrix[3][2],matrix[3][3])

    # helper function to convert a vector to a tuple
    def vec3_to_tuple(vec):
        return (vec[0],vec[1],vec[2])

    # get the scene object from dependency graph
    scene = depsgraph.scene

    # this is a special code for the render to identify that the serialized input is still valid.
    vericiation_bits = SID('verification bits')
    fs.serialize( vericiation_bits )

    # camera node
    camera = scene.camera
    if camera is None:
        print("There is no active camera.")
        return

    camera_data = camera.data

    pos, target, up = lookat_camera(camera)
    sensor_w = camera_data.sensor_width
    sensor_h = camera_data.sensor_height
    sensor_fit = 0.0 # auto
    sfit = camera_data.sensor_fit
    if sfit == 'VERTICAL':
        sensor_fit = 2.0
    elif sfit == 'HORIZONTAL':
        sensor_fit = 1.0
    aspect_ratio_x = scene.render.pixel_aspect_x
    aspect_ratio_y = scene.render.pixel_aspect_y
    fov_angle = camera_data.angle

    # resolution of the final image
    xres = scene.render.resolution_x * scene.render.resolution_percentage / 100
    yres = scene.render.resolution_y * scene.render.resolution_percentage / 100

    fs.serialize(SID('PerspectiveCameraEntity'))
    fs.serialize(vec3_to_tuple(pos))
    fs.serialize(vec3_to_tuple(up))
    fs.serialize(vec3_to_tuple(target))
    fs.serialize((int(xres),int(yres)))
    fs.serialize(camera.data.sort_data.lens_size)
    fs.serialize((sensor_w,sensor_h))
    fs.serialize(int(sensor_fit))
    fs.serialize((aspect_ratio_x,aspect_ratio_y))
    fs.serialize(fov_angle)

    all_lights = [ ob for ob in depsgraph_objects(depsgraph) if ob.type == 'LIGHT' ]
    all_objs = [ ob for ob in depsgraph_objects(depsgraph) if ob.type == 'MESH' ]

    total_vert_cnt = 0
    total_prim_cnt = 0
    # export meshes
    for obj in all_objs:
        fs.serialize(SID('VisualEntity'))
        fs.serialize( matrix_to_tuple( MatrixBlenderToSort() @ obj.matrix_world ) )
        fs.serialize( 1 )   # only one mesh for each mesh entity
        stat = None
        # apply the modifier if there is one
        if obj.type != 'MESH' or obj.is_modified(scene, 'RENDER'):
            try:
                evaluated_obj = obj.evaluated_get(depsgraph)
                mesh = evaluated_obj.to_mesh()
                stat = export_mesh(evaluated_obj, mesh, fs)
            finally:
                evaluated_obj.to_mesh_clear()
        else:
            stat = export_mesh(obj, obj.data, fs)

        total_vert_cnt += stat[0]
        total_prim_cnt += stat[1]

    # output hair/fur exporting
    for obj in all_objs:
        evaluted_obj = obj.evaluated_get(depsgraph)

        # output hair/fur information
        if len( evaluted_obj.particle_systems ) > 0:
            fs.serialize( SID('VisualEntity') )
            fs.serialize( matrix_to_tuple( MatrixBlenderToSort() @ evaluted_obj.matrix_world ) )
            fs.serialize( len( evaluted_obj.particle_systems ) )
            for ps in evaluted_obj.particle_systems:
                stat = export_hair( ps , evaluted_obj , scene , is_preview, fs )
                total_vert_cnt += stat[0]
                total_prim_cnt += stat[1]

    log( "Total vertices: %d." % total_vert_cnt )
    log( "Total primitives: %d." % total_prim_cnt )

    mapping = {'SUN': 'DirLightEntity', 'POINT': 'PointLightEntity', 'SPOT': 'SpotLightEntity', 'AREA': 'AreaLightEntity' }
    for ob in all_lights:
        lamp = ob.data

        # This matrix will be used to transform data in SORT coodinate. So it needs to start from SORT coodinate system instead of Blender's.
        # light faces forward Y+ in SORT, while it faces Z- in Blender, needs to flip the direction
        flip_mat = mathutils.Matrix([[ 1.0 , 0.0 , 0.0 , 0.0 ] , [ 0.0 , -1.0 , 0.0 , 0.0 ] , [ 0.0 , 0.0 , 1.0 , 0.0 ] , [ 0.0 , 0.0 , 0.0 , 1.0 ]])
        world_matrix = MatrixBlenderToSort() @ ob.matrix_world @ MatrixSortToBlender() @ flip_mat

        # make sure the type of the light is supported
        assert( lamp.type in mapping )

        # name identifier of the light
        fs.serialize( SID(mapping[lamp.type]) )

        # transformation of light source
        fs.serialize(matrix_to_tuple(world_matrix))

        # total light power, it defines how bright light is
        fs.serialize(lamp.energy)

        # light spectrum color, it defines color of the light
        fs.serialize(lamp.color[:])

        # spot light and area light have extra properties to be serialized
        if lamp.type == 'SPOT':
            falloff_start = degrees(lamp.spot_size * ( 1.0 - lamp.spot_blend ) * 0.5)
            falloff_range = degrees(lamp.spot_size*0.5)
            fs.serialize(falloff_start)
            fs.serialize(falloff_range)
        elif lamp.type == 'AREA':
            fs.serialize( SID(lamp.shape) )
            if lamp.shape == 'SQUARE':
                fs.serialize(lamp.size)
            elif lamp.shape == 'RECTANGLE':
                fs.serialize(lamp.size)
                fs.serialize(lamp.size_y)
            elif lamp.shape == 'DISK':
                fs.serialize(lamp.size * 0.5)

    hdr_sky_color = scene.sort_hdr_sky.ambient_sky_color
    hdr_sky_intensity = scene.sort_hdr_sky.ambient_sky_intensity
    hdr_sky_image = scene.sort_hdr_sky.hdr_image
    if hdr_sky_intensity > 0.0 and ( hdr_sky_color[0] > 0.0 or hdr_sky_color[1] > 0.0 or hdr_sky_color[2] > 0.0 ):
        fs.serialize(SID('SkyLightEntity'))
        global_matrix = mathutils.Matrix()
        fs.serialize(matrix_to_tuple(global_matrix))
        fs.serialize(hdr_sky_intensity)
        fs.serialize(hdr_sky_color[:])

        if hdr_sky_image is None:
            fs.serialize('')
        else:
            fs.serialize(bpy.path.abspath( hdr_sky_image.filepath ))

    # to indicate the scene stream comes to an end
    fs.serialize(SID('End of Entities'))

    # the accelerator
    sort_data = scene.sort_data
    accelerator_type = sort_data.accelerator_type_prop
    if accelerator_type == "bvh":
        fs.serialize( SID('Bvh') )
        fs.serialize( int(sort_data.bvh_max_node_depth) )
        fs.serialize( int(sort_data.bvh_max_pri_in_leaf) )
    elif accelerator_type == "KDTree":
        fs.serialize( SID('KDTree') )
        fs.serialize( int(sort_data.kdtree_max_node_depth) )
        fs.serialize( int(sort_data.kdtree_max_pri_in_leaf) )
    elif accelerator_type == "OcTree":
        fs.serialize( SID('OcTree') )
        fs.serialize( int(sort_data.octree_max_node_depth) )
        fs.serialize( int(sort_data.octree_max_pri_in_leaf) )
    elif accelerator_type == "Qbvh":
        fs.serialize( SID('Qbvh') )
        fs.serialize( int(sort_data.qbvh_max_node_depth) )
        fs.serialize( int(sort_data.qbvh_max_pri_in_leaf) )
    elif accelerator_type == "Obvh":
        fs.serialize( SID('Obvh') )
        fs.serialize( int(sort_data.obvh_max_node_depth) )
        fs.serialize( int(sort_data.obvh_max_pri_in_leaf) )
    elif accelerator_type == "UniGrid":
        fs.serialize( SID('UniGrid') )
    elif accelerator_type == "Embree":
        fs.serialize( SID('Embree'))

# avoid having space in material name
def name_compat(name):
    if name is None:
        return 'None'
    else:
        return name.replace(' ', '_')

# export glocal settings for the renderer
def export_global_config(scene, fs, sort_resource_path):
    # global renderer configuration
    xres = scene.render.resolution_x * scene.render.resolution_percentage / 100
    yres = scene.render.resolution_y * scene.render.resolution_percentage / 100

    sort_data = scene.sort_data

    integrator_type = sort_data.integrator_type_prop
    
    fs.serialize( 0 )
    fs.serialize( sort_resource_path )
    fs.serialize( int(sort_data.thread_num_prop) )
    fs.serialize( int(sort_data.sampler_count_prop) )
    fs.serialize( int(xres) )
    fs.serialize( int(yres) )
    fs.serialize( sort_data.clampping )

    fs.serialize( SID(integrator_type) )
    fs.serialize( int(sort_data.inte_max_recur_depth) )
    if integrator_type == "PathTracing":
        fs.serialize( int(sort_data.max_bssrdf_bounces) )
    if integrator_type == "AmbientOcclusion":
        fs.serialize( sort_data.ao_max_dist )
    if integrator_type == "BidirPathTracing" or integrator_type == "LightTracing":
        fs.serialize( bool(sort_data.bdpt_mis) )
    if integrator_type == "InstantRadiosity":
        fs.serialize( sort_data.ir_light_path_set_num )
        fs.serialize( sort_data.ir_light_path_num )
        fs.serialize( sort_data.ir_min_dist )

# export smoke information
def export_smoke(obj, fs):
    smoke_modifier = get_smoke_modifier(obj)
    if not smoke_modifier:
        fs.serialize( SID('no_volume') )
        return
    
    # making sure there is density data
    domain = smoke_modifier.domain_settings
    if len(domain.density_grid) == 0:
        fs.serialize( SID('no_volume') )
        return

    fs.serialize( SID('has_volume') )
    
    # dimension of the volume data
    x, y, z = domain.domain_resolution
    fs.serialize(x)
    fs.serialize(y)
    fs.serialize(z)

    # the color itself, don't export it for now
    # color_grid = np.fromiter(domain.color_grid, dtype=np.float32)
    # fs.serialize(color_grid)

    FLTFMT = struct.Struct('=f')

    # the density itself
    density_data = bytearray()
    density_grid = np.fromiter(domain.density_grid, dtype=np.float32)
    for density in density_grid:
        density_data += FLTFMT.pack(density)

    fs.serialize(density_data)

# export a mesh
def export_mesh(obj, mesh, fs):
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
    wo3_tris = bytearray()

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

    # Warning this function seems to cause quite some trouble on MacOS during the first renderer somehow.
    # And this problem only exists on MacOS not the other two OS.
    # Since there is not a low hanging fruit solution for now, it is disabled by default
    # generate tangent if there is UV, there seems to always be true in Blender 2.8, but not in 2.7x
    #if has_uv:
    #    mesh.calc_tangents( uvmap = uv_layer_name )

    vert_cnt = 0
    remapping = {}

    mesh_sid = SID( mesh.name )

    for poly in mesh.polygons:
        smooth = poly.use_smooth
        normal = poly.normal[:]

        oi = []
        for loop_index in range(poly.loop_start, poly.loop_start + poly.loop_total):
            # vertex index
            vid = mesh.loops[loop_index].vertex_index
            # vertex information
            vert = verts[vid]

            # uv coordinate
            uvcoord = uv_layer[loop_index].uv[:] if has_uv else ( 0.0 , 0.0 )

            # use smooth normal if necessary
            if smooth:
                normal = vert.normal[:]

            #tangent = mesh.loops[loop_index].tangent

            # an unique key to identify the vertex
            key = (vid, loop_index, smooth)

            # acquire the key if possible, otherwise pack one
            out_idx = remapping.get(key)
            if out_idx is None:
                out_idx = vert_cnt
                remapping[key] = out_idx
                wo3_verts += VERTFMT.pack(vert.co[0], vert.co[1], vert.co[2], normal[0], normal[1], normal[2], uvcoord[0], uvcoord[1])
                vert_cnt += 1
            oi.append(out_idx)

        matid = -1
        matname = name_compat(material_names[poly.material_index]) if len( material_names ) > 0 else None
        matid = matname_to_id[matname] if matname in matname_to_id else -1
        if len(oi) == 3:
            # triangle
            wo3_tris += TRIFMT.pack(oi[0], oi[1], oi[2], matid)
            primitive_cnt += 1
        elif len(oi) == 4:
            # quad
            wo3_tris += TRIFMT.pack(oi[0], oi[1], oi[2], matid)
            wo3_tris += TRIFMT.pack(oi[0], oi[2], oi[3], matid)
            primitive_cnt += 2
        else:
            # no other primitive supported in mesh
            # assert( False )
            log("Warning, there is unsupported geometry. The exported scene may be incomplete.")

    fs.serialize(SID('MeshVisual'))
    fs.serialize(bool(has_uv))
    fs.serialize(LENFMT.pack(vert_cnt))
    fs.serialize(wo3_verts)
    fs.serialize(LENFMT.pack(primitive_cnt))
    fs.serialize(wo3_tris)

    # export smoke data if needed, this is for volumetric rendering
    export_smoke(obj, fs)

    fs.serialize(SID('end of mesh'))

    return (vert_cnt, primitive_cnt)

# export hair information
def export_hair(ps, obj, scene, is_preview, fs):
    LENFMT = struct.Struct('=i')
    POINTFMT = struct.Struct('=fff')

    vert_cnt = 0
    hair_step = ps.settings.display_step if is_preview else ps.settings.render_step
    width_tip = ps.settings.sort_data.fur_tip
    width_bottom = ps.settings.sort_data.fur_bottom

    # extract the material of the hair
    mat_local_index = ps.settings.material
    mat_index = 0xffffffff

    if mat_local_index > 0 and mat_local_index <= len( obj.data.materials ):
        mat_name = name_compat(obj.data.materials[mat_local_index-1].name)
        mat_index = matname_to_id[mat_name] if mat_name in matname_to_id else 0xffffffff

    # for some unknown reason
    steps = 2 ** hair_step

    verts = bytearray()

    world2Local = obj.matrix_world.inverted()
    num_parents = len( ps.particles )
    num_children = len( ps.child_particles )

    hair_cnt = num_parents + num_children
    total_hair_segs = 0

    real_hair_cnt = 0
    for pindex in range(hair_cnt):
        hair = []
        for step in range(0, steps + 1):
            co = ps.co_hair(obj, particle_no = pindex, step = step)

            if co[0] == 0 and co[1] == 0 and co[2] == 0:
                continue

            co = world2Local @ co
            hair.append( co )
            vert_cnt += 1

        if len(hair) <= 1:
            continue

        real_hair_cnt += 1

        verts += LENFMT.pack( len(hair) - 1 )
        for h in hair :
            verts += POINTFMT.pack( h[0] , h[1] , h[2] )
        total_hair_segs += len(hair) - 1

    fs.serialize( SID('HairVisual') )
    fs.serialize( real_hair_cnt )
    fs.serialize( width_tip )
    fs.serialize( width_bottom )
    fs.serialize( mat_index )
    fs.serialize( verts )

    return (vert_cnt, total_hair_segs)

# find the output node, duplicated code, to be cleaned
def find_output_node(material):
    if material and material.sort_material:
        ntree = material.sort_material
        for node in ntree.nodes:
            if getattr(node, "bl_idname", None) == 'SORTNodeOutput':
                return node
    return None

# get the from node of this socket if there is one recursively
def get_from_socket(socket):
    if not socket.is_linked:
        return None

    # there should be exactly one link attached to an input socket, this is guaranteed by Blender
    assert( len( socket.links ) == 1 )
    other = socket.links[0].from_socket
    if other.node.bl_idname == 'NodeReroute':
        return get_from_socket(other.node.inputs[0])
    return other

# This function will iterate through all visited nodes in the scene and populate everything in a hash table
# Apart from collecting shaders, it will also collect all heavy data, like measured BRDF data, texture.
def collect_shader_resources(depsgraph, scene, fs):
    # don't output any osl_shaders if using default materials
    if scene.sort_data.allUseDefaultMaterial is True:
        fs.serialize( 0 )
        return None

    resources = []
    visited = set()
    for material in list_materials(depsgraph):
        # get output nodes
        output_node = find_output_node(material)
        if output_node is None:
            continue

        def collect_resources(mat_node, visited, resources):
            if mat_node is None:
                return
            
            # no need to iterate a node twice
            if mat_node in visited:
                return
            visited.add(mat_node)
            
            # iterate through all source shader nodes.
            for socket in mat_node.inputs:
                input_socket = get_from_socket(socket)  # this is a temporary solution
                if input_socket is None:
                    continue
                collect_resources(input_socket.node, visited, resources)
            
            # if it is a shader group node, recursively iterate its nodes
            if mat_node.isGroupNode():
                # sub tree for the group nodes
                sub_tree = mat_node.getGroupTree()

                # recursively parse the node first
                output_node = sub_tree.nodes.get("Group Outputs")

                # recursively collect more nodes
                collect_resources(output_node, visited, resources)
            else:
                # otherwise simply populate the resources in this node
                mat_node.populateResources(resources)

        collect_resources(output_node, visited, resources)

    fs.serialize( len( resources ) )
    for resource in resources:
        fs.serialize( resource[0] ) # type
        fs.serialize( resource[1] ) # external file name

matname_to_id = {}
def export_materials(depsgraph, fs):
    # if we are in no-material mode, just skip outputting all materials
    if depsgraph.scene.sort_data.allUseDefaultMaterial is True:
        fs.serialize( SID('End of Material') )
        return None

    # this is used to keep track of all visited nodes to avoid duplicated nodes exported.
    visited_shader_unit_types = set()

    # loop through all materials and output them if valid
    i = 0
    materials = list_materials(depsgraph)
    for material in materials:
        # indicating material exporting
        logD( 'Exporting material %s.' %(material.name) )

        # get output nodes
        output_node = find_output_node(material)
        if output_node is None:
            logD( 'Material %s doesn\'t have any output node, it is invalid and will be ignored.' %(material.name) )
            continue
        
        # update the material mapping
        compact_material_name = name_compat(material.name)
        matname_to_id[compact_material_name] = i
        i += 1

        # whether the material has transparent node
        has_transparent_node = False
        # whether there is sss in the material
        has_sss_node = False

        # basically, this is a topological sort to serialize all nodes.
        # each node type will get exported exactly once to avoid duplicated shader unit compliation.
        def collect_shader_unit(shader_node, visited_instance, visited_types, shader_node_connections, node_type_mapping, input_index = -1):
            # no need to process a node multiple times
            if shader_node in visited_node_instances:
                return

            # add the current node to visited cache to avoid it being visited again
            if shader_node.isMaterialOutputNode() is False:
                visited_node_instances.add(shader_node)

            # update transparent and sss flag
            if shader_node.isTransparentNode() is True:
                nonlocal has_transparent_node
                has_transparent_node = True
            if shader_node.isSSSNode() is True:
                nonlocal has_sss_node
                has_sss_node = True

            # this identifies the unique name of the shader
            current_shader_node_name = shader_node.getUniqueName()

            # output node is a bit special that it can be revisited
            if shader_node.isMaterialOutputNode():
                current_shader_node_name = current_shader_node_name + compact_material_name

            # the type of the node
            shader_node_type = shader_node.type_identifier()

            # mapping from node name to node type
            node_type_mapping[shader_node] = shader_node_type

            # grab all source shader nodes
            inputs = shader_node.inputs 
            if input_index >= 0:
                # out of index, simply return, this is because some old assets doesn't have the volume channel
                # a bit tolerance will allow me to still use the render with old assets
                if input_index >= len(shader_node.inputs):
                    return
                else:
                    inputs = [shader_node.inputs[input_index]]

            for socket in inputs:
                input_socket = get_from_socket( socket )  # this is a temporary solution
                if input_socket is None:
                    continue
                source_node = input_socket.node

                source_param = source_node.getShaderOutputParameterName(input_socket.name)
                target_param = shader_node.getShaderInputParameterName(socket.name)

                source_shader_node_name = source_node.getUniqueName()

                # add the shader unit connection
                shader_node_connections.append( ( source_shader_node_name , source_param , current_shader_node_name, target_param ) )

                # recursively collect shader unit
                collect_shader_unit(source_node, visited_node_instances, visited_types, shader_node_connections, node_type_mapping)

            # no need to serialize the same node multiple times
            if shader_node_type in visited_types:
                return
            visited_types.add(shader_node_type)

            # export the shader node
            if shader_node.isGroupNode():
                # shader group should have a new set of connections
                shader_group_connections = []
                # shader group should also has its own node mapping
                shader_group_node_mapping = {}
                # start from a new visited cache
                shader_group_node_visited = set()   
                
                # sub tree for the group nodes
                sub_tree = shader_node.getGroupTree()

                # recursively parse the node first
                output_node = sub_tree.nodes.get("Group Outputs")
                collect_shader_unit(output_node, shader_group_node_visited, visited_types, shader_group_connections, shader_group_node_mapping)

                # it is important to visit the input node even if it is not connected since this needs to be connected with exposed arguments.
                # lacking this node will result in tsl compilation error
                input_node = sub_tree.nodes.get("Group Inputs")
                collect_shader_unit(input_node, shader_group_node_visited, visited_types, shader_group_connections, shader_group_node_mapping)

                # start serialization
                fs.serialize(SID("ShaderGroupTemplate"))
                fs.serialize(shader_node_type)

                fs.serialize(len(shader_group_node_mapping))
                for shader_node, shader_type in shader_group_node_mapping.items():
                    fs.serialize(shader_node.getUniqueName())
                    fs.serialize(shader_type)
                    shader_node.serialize_prop(fs)
                fs.serialize(len(shader_group_connections))
                for connection in shader_group_connections:
                    fs.serialize( connection[0] )
                    fs.serialize( connection[1] )
                    fs.serialize( connection[2] )
                    fs.serialize( connection[3] )
                
                # indicate the exposed arguments
                output_node.serialize_exposed_args(fs)

                # if there is input node, exposed the inputs
                input_node = sub_tree.nodes.get("Group Inputs")
                if input_node is not None:
                    input_node.serialize_exposed_args(fs)
                else:
                    fs.serialize( "" )
            else:
                fs.serialize(SID('ShaderUnitTemplate'))
                fs.serialize(shader_node_type)
                fs.serialize(shader_node.generate_osl_source())
                shader_node.serialize_shader_resource(fs)

        # this is the shader node connections
        surface_shader_node_connections = []
        volume_shader_node_connections = []

        # this hash table keeps track of all visited shader node instance
        visited_node_instances = set()

        # node type mapping, this maps from node name to node type
        surface_shader_node_type = {}
        volume_shader_node_type = {}

        # iterate the material for surface shader
        collect_shader_unit(output_node, visited_node_instances, visited_shader_unit_types, surface_shader_node_connections, surface_shader_node_type, 0)
        # iterate the material for volume shader
        collect_shader_unit(output_node, visited_node_instances, visited_shader_unit_types, volume_shader_node_connections, volume_shader_node_type, 1)

        # serialize this material, it is a real material
        fs.serialize(SID('Material'))
        fs.serialize(compact_material_name)

        if len(surface_shader_node_type) > 1:
            fs.serialize(SID('Surface Shader'))
            fs.serialize(len(surface_shader_node_type))
            for shader_node, shader_type in surface_shader_node_type.items():
                fs.serialize(shader_node.getUniqueName())
                fs.serialize(shader_type)
                shader_node.serialize_prop(fs)
            fs.serialize(len(surface_shader_node_connections))
            for connection in surface_shader_node_connections:
                fs.serialize( connection[0] )
                fs.serialize( connection[1] )
                fs.serialize( connection[2] )
                fs.serialize( connection[3] )
        else:
            fs.serialize( SID('Invalid Surface Shader') )

        if len(volume_shader_node_type) > 1 :
            fs.serialize(SID('Volume Shader'))
            fs.serialize(len(volume_shader_node_type))
            for shader_node, shader_type in volume_shader_node_type.items():
                fs.serialize(shader_node.getUniqueName())
                fs.serialize(shader_type)
                shader_node.serialize_prop(fs)
            fs.serialize(len(volume_shader_node_connections))
            for connection in volume_shader_node_connections:
                fs.serialize( connection[0] )
                fs.serialize( connection[1] )
                fs.serialize( connection[2] )
                fs.serialize( connection[3] )
        else:
            fs.serialize( SID('Invalid Volume Shader') )

        # mark whether there is transparent support in the material, this is very important because it will affect performance eventually.
        fs.serialize( bool(has_transparent_node) )
        fs.serialize( bool(has_sss_node) )

        # volume step size and step count
        fs.serialize( material.sort_material.volume_step )
        fs.serialize( material.sort_material.volume_step_cnt )

    # indicate the end of material parsing
    fs.serialize(SID('End of Material'))