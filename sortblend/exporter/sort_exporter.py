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
from math import degrees
from . import exporter_common
import xml.etree.cElementTree as ET
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
        return_path = get_sort_dir() + "/intermediate/"
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
    # create root node
    root = ET.Element("Root")

    # create immediate file path
    create_path(scene, force_debug)
    # export sort file
    export_sort_file(scene, root, force_debug)
    # export material
    export_material(scene, root, force_debug)
    # export scene
    export_scene(scene, root, force_debug)

    # output the xml
    output_sort_file = get_intermediate_dir(force_debug) + 'sort_scene.xml'
    tree = ET.ElementTree(root)
    tree.write(output_sort_file)
    print( 'SORT Scene File:\t' + output_sort_file )

# clear old data and create new path
def create_path(scene, force_debug):
    global intermediate_dir
    intermediate_dir = tempfile.mkdtemp(suffix='w')
    # get immediate directory
    output_dir = get_intermediate_dir(force_debug)
    
    # clear the old directory
    if os.path.exists(output_dir):
        shutil.rmtree(output_dir)
    # create one if there is no such directory
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)

# open sort file
def export_sort_file(scene, root, force_debug):
    # the scene node
    output_dir = get_intermediate_dir(force_debug)

    # resource path node
    output_dir = get_intermediate_dir(force_debug)
    ET.SubElement(root , 'Resource', path= output_dir )

    # the integrator node
    integrator_type = scene.integrator_type_prop
    integrator_node = ET.SubElement(root, 'Integrator', type=integrator_type)
    ET.SubElement( integrator_node , "Property" , name="inte_max_recur_depth" , value="%d"%scene.inte_max_recur_depth )
    if integrator_type == "ao":
        ET.SubElement( integrator_node , "Property" , name="max_distance" , value="%f"%scene.ao_max_dist)
    if integrator_type == "bdpt":
        ET.SubElement( integrator_node , "Property" , name="bdpt_mis" , value="%d"%scene.bdpt_mis)
    if integrator_type == 'ir':
        ET.SubElement( integrator_node , "Property" , name="light_path_set_num" , value='%d'%scene.ir_light_path_set_num)
        ET.SubElement( integrator_node , "Property" , name="light_path_num" , value='%d'%scene.ir_light_path_num)
        ET.SubElement( integrator_node , "Property" , name="min_distance" , value='%f'%scene.ir_min_dist)
    # image size
    xres = scene.render.resolution_x * scene.render.resolution_percentage / 100
    yres = scene.render.resolution_y * scene.render.resolution_percentage / 100
    ET.SubElement(root, 'RenderTargetSize', w='%d'%xres, h='%d'%yres )
    # output file name
    ET.SubElement(root, 'OutputFile', name='blender_generated.exr')
    # sampler type
    sampler_type = scene.sampler_type_prop
    sampler_count = scene.sampler_count_prop
    ET.SubElement(root, 'Sampler', type=sampler_type, round='%s'%sampler_count)
    # camera node
    camera = exporter_common.getCamera(scene)
    pos, target, up = lookAtSORT(camera)
    camera_node = ET.SubElement(root, 'Camera', type='perspective')
    ET.SubElement( camera_node , "Property" , name="eye" , value=exporter_common.vec3tostr(pos))
    ET.SubElement( camera_node , "Property" , name="up" , value=exporter_common.vec3tostr(up))
    ET.SubElement( camera_node , "Property" , name="target" , value=exporter_common.vec3tostr(target))
    ET.SubElement( camera_node , "Property" , name="len" , value='%f'%camera.data.sort_camera.sort_camera_lens.lens_size)
    ET.SubElement( camera_node , "Property" , name="interaxial" , value="0")
    ET.SubElement( camera_node , "Property" , name="width" , value="0")
    ET.SubElement( camera_node , "Property" , name="height" , value="0")
    sensor_w = bpy.data.cameras[0].sensor_width
    sensor_h = bpy.data.cameras[0].sensor_height
    sensor_fit = 0.0 # auto
    sfit = bpy.data.cameras[0].sensor_fit
    if sfit == 'VERTICAL':
        sensor_fit = 2.0
    elif sfit == 'HORIZONTAL':
        sensor_fit = 1.0
    ET.SubElement( camera_node , "Property" , name="sensorsize" , value= "%s %s %f"%(sensor_w,sensor_h,sensor_fit))
    aspect_ratio_x = scene.render.pixel_aspect_x
    aspect_ratio_y = scene.render.pixel_aspect_y
    ET.SubElement( camera_node , "Property" , name="aspect" , value="%s %s"%(aspect_ratio_x,aspect_ratio_y))
    fov_angle = bpy.data.cameras[0].angle
    ET.SubElement( camera_node , "Property" , name="fov" , value= "%s"%fov_angle)
    camera_shift_x = bpy.data.cameras[0].shift_x
    camera_shift_y = bpy.data.cameras[0].shift_y
    ET.SubElement( camera_node , "Property" , name="shift" , value="%s %s"%(camera_shift_x,camera_shift_y))
    # output thread num
    thread_num = scene.thread_num_prop
    ET.SubElement( root , 'ThreadNum', name='%s'%thread_num)

# export scene
def export_scene(scene, root, force_debug):
    scene_root = ET.SubElement( root , 'Scene' )

    # acceleration structure
    accelerator_type = scene.accelerator_type_prop
    ET.SubElement( scene_root , 'Accel', type=accelerator_type)

    for ob in exporter_common.getMeshList(scene):
        model_node = ET.SubElement( scene_root , 'Model' , filename=ob.name + '.obj', name = ob.name )
        transform_node = ET.SubElement( model_node , 'Transform' )
        ET.SubElement( transform_node , 'Matrix' , value = 'm '+ exporter_common.matrixtostr( MatrixBlenderToSort() * ob.matrix_world) )
        # output the mesh to file
        export_mesh(ob,scene, force_debug)

    for ob in exporter_common.getLightList(scene):
        lamp = ob.data
        # light faces forward Y+ in SORT, while it faces Z- in Blender, needs to flip the direction
        flip_mat = mathutils.Matrix([[ 1.0 , 0.0 , 0.0 , 0.0 ] , [ 0.0 , -1.0 , 0.0 , 0.0 ] , [ 0.0 , 0.0 , 1.0 , 0.0 ] , [ 0.0 , 0.0 , 0.0 , 1.0 ]])
        world_matrix = MatrixBlenderToSort() * ob.matrix_world * MatrixSortToBlender() * flip_mat
        if lamp.type == 'SUN':
            light_node = ET.SubElement( scene_root , 'Light' , type='distant')
            light_spectrum = np.array(lamp.color[:])
            light_spectrum *= lamp.energy
            ET.SubElement( light_node , 'Property' , name='intensity' , value=exporter_common.vec3tostr(light_spectrum))
            ET.SubElement( light_node , 'Property' , name='transform' , value = "m " + exporter_common.matrixtostr( world_matrix ) )                
        elif lamp.type == 'POINT':
            light_node = ET.SubElement( scene_root , 'Light' , type='point')
            light_spectrum = np.array(lamp.color[:])
            light_spectrum *= lamp.energy
            light_position = world_matrix.col[3]
            ET.SubElement( light_node , 'Property' , name='intensity' , value=exporter_common.vec3tostr(light_spectrum))
            ET.SubElement( light_node , 'Property' , name='transform' , value = "m " + exporter_common.matrixtostr( world_matrix ) )                
        elif lamp.type == 'SPOT':
            light_node = ET.SubElement( scene_root , 'Light' , type='spot')
            light_spectrum = np.array(lamp.color[:])
            light_spectrum *= lamp.energy
            light_dir = world_matrix.col[2] * -1.0
            light_position = world_matrix.col[3]
            ET.SubElement( light_node , 'Property' , name='transform' , value = "m " + exporter_common.matrixtostr( world_matrix ) )
            ET.SubElement( light_node , 'Property' , name='intensity' , value=exporter_common.vec3tostr(light_spectrum))
            ET.SubElement( light_node , 'Property' , name='falloff_start' ,value="%d"%(degrees(lamp.spot_size * ( 1.0 - lamp.spot_blend ) * 0.5)))
            ET.SubElement( light_node , 'Property' , name='range' ,value="%d"%(degrees(lamp.spot_size*0.5)))
        elif lamp.type == 'AREA':
            light_node = ET.SubElement( scene_root , 'Light' , type='area')
            light_spectrum = np.array(lamp.color[:])
            light_spectrum *= lamp.energy
            light_dir = world_matrix.col[2] * -1.0
            light_position = world_matrix.col[3]

            sizeX = lamp.size
            sizeY = lamp.size_y
            shape = 'square'
            if lamp.shape == 'RECTANGLE':
                shape = 'rectangle'
            
            ET.SubElement( light_node , 'Property' , name='transform' , value = "m " + exporter_common.matrixtostr(world_matrix) )
            ET.SubElement( light_node , 'Property' , name='shape' ,value=shape)
            ET.SubElement( light_node , 'Property' , name='intensity' , value=exporter_common.vec3tostr(light_spectrum))
            ET.SubElement( light_node , 'Property' , name='sizex' ,value='%f'%sizeX )
            ET.SubElement( light_node , 'Property' , name='sizey' ,value='%f'%sizeY )
        elif lamp.type == 'HEMI':
            light_spectrum = np.array(lamp.color[:])
            light_spectrum *= lamp.energy
            light_node = ET.SubElement( scene_root , 'Light' , type='skylight')
            ET.SubElement( light_node , 'Property' , name='intensity' , value=exporter_common.vec3tostr(light_spectrum))
            ET.SubElement( light_node , 'Property' , name='transform' , value = "m " + exporter_common.matrixtostr( MatrixBlenderToSort() * ob.matrix_world * MatrixSortToBlender() ) )
            ET.SubElement( light_node , 'Property' , name='type' ,value='sky_sphere')
            ET.SubElement( light_node , 'Property' , name='image' ,value= bpy.path.abspath( lamp.sort_lamp.sort_lamp_hemi.envmap_file ) )

mtl_dict = {}
mtl_rev_dict = {}

def name_compat(name):
    if name is None:
        return 'None'
    else:
        return name.replace(' ', '_')

# export mesh file
def export_mesh(obj,scene,force_debug):
    output_path = get_intermediate_dir(force_debug) + obj.name + '.obj'

    # the mesh object
    mesh = obj.data

    faceuv = len(mesh.uv_textures) > 0
    if faceuv:
        uv_layer = mesh.uv_layers.active.data[:]

    # face index pairs
    face_index_pairs = [(face, index) for index, face in enumerate(mesh.polygons)]

    # generate normal data
    mesh.calc_normals_split()
    with open(output_path, 'w') as file:
        file.write("# OBJ file\n")

        contextMat = None
        materials = mesh.materials[:]
        material_names = [m.name if m else None for m in materials]

        # avoid bad index errors
        if not materials:
            materials = [None]
            material_names = [name_compat(None)]

        name1 = obj.name
        name2 = obj.data.name
        if name1 == name2:
            obnamestring = name_compat(name1)
        else:
            obnamestring = '%s_%s' % (name_compat(name1), name_compat(name2))

        file.write('g %s\n' % obnamestring)

        # output vertices
        for v in mesh.vertices:
            file.write("v %.4f %.4f %.4f\n" % (v.co[:]))

        # UV
        uv_unique_count = no_unique_count = 0
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
                    uv_key = round(uv[0], 4), round(uv[1], 4)
                    uv_val = uv_get(uv_key)
                    if uv_val is None:
                        uv_val = uv_dict[uv_key] = uv_unique_count
                        file.write('vt %.6f %.6f\n' % uv[:])
                        uv_unique_count += 1
                    uv_ls.append(uv_val)

            del uv_dict, uv, f_index, uv_index, uv_ls, uv_get, uv_key, uv_val

        # output normal
        no_key = no_val = None
        normals_to_idx = {}
        no_get = normals_to_idx.get
        no_unique_count = 0
        loops_to_normals = [0] * len(mesh.loops)
        for f, f_index in face_index_pairs:
            for l_idx in f.loop_indices:
                def veckey3d(v):
                    return round(v.x, 4), round(v.y, 4), round(v.z, 4)
                no_key = veckey3d(mesh.loops[l_idx].normal)
                no_val = no_get(no_key)
                if no_val is None:
                    no_val = normals_to_idx[no_key] = no_unique_count
                    file.write('vn %.6f %.6f %.6f\n' % no_key)
                    no_unique_count += 1
                loops_to_normals[l_idx] = no_val
        del normals_to_idx, no_get, no_key, no_val

        me_verts = mesh.vertices

        for f, f_index in face_index_pairs:
            f_smooth = f.use_smooth
            f_mat = min(f.material_index, len(materials) - 1)

            key = material_names[f_mat], None  # No image, use None instead.

            # CHECK FOR CONTEXT SWITCH
            if key == contextMat:
                pass  # Context already switched, dont do anything
            else:
                if key[0] is None and key[1] is None:
                    # Write a null material, since we know the context has changed.
                    #if EXPORT_GROUP_BY_MAT:
                    # can be mat_image or (null)
                    file.write("g %s_%s\n" % (name_compat(obj.name), name_compat(obj.data.name)))
                    file.write("usemtl (null)\n")
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

                    file.write("g %s_%s_%s\n" % (name_compat(obj.name), name_compat(obj.data.name), mat_data[0]))
                    file.write("usemtl %s\n" % mat_data[0])

            # update current context material
            contextMat = key

            # output face information
            f_v = [(vi, me_verts[v_idx], l_idx)
                       for vi, (v_idx, l_idx) in enumerate(zip(f.vertices, f.loop_indices))]
            file.write("f")

            totverts = 1
            totuvco = 1
            totno = 1
            if faceuv:
                for vi, v, li in f_v:
                    file.write(" %d/%d/%d" % (totverts + v.index,
                                      totuvco + uv_face_mapping[f_index][vi],
                                      totno + loops_to_normals[li],
                                      ))  # vert, uv, normal
            else:  # No UV's
                for vi, v, li in f_v:
                    file.write(" %d//%d" % (totverts + v.index, totno + loops_to_normals[li]))

            file.write("\n")

def export_material(scene, root, force_debug):
    mat_root = ET.SubElement( root , 'Materials' )

    for material in exporter_common.getMaterialList(scene):
        # get the sort tree nodes
        ntree = bpy.data.node_groups[material.sort_material.sortnodetree]

        # find the output node, duplicated code, to be cleaned
        def find_output_node(material):
            if material and material.sort_material and material.sort_material.sortnodetree:
                ntree = bpy.data.node_groups[material.sort_material.sortnodetree]
                for node in ntree.nodes:
                    if getattr(node, "bl_idname", None) == 'SORTNodeOutput':
                        return node
            return None

        # get output nodes
        output_node = find_output_node(material)
        if output_node is None:
            continue

        print( 'Exporting material: ' + material.name )

        # material node
        mat_node = ET.SubElement( mat_root , 'Material', name=name_compat(material.name) )
        
        def export_props(mat_node , xml_node):
            export_prop_in_sort = lambda n , v : ET.SubElement( xml_node , 'Property' , name=n , value=v )
            mat_node.export_sort(export_prop_in_sort)

            inputs = mat_node.inputs
            for socket in inputs:
                if socket.is_linked:
                    def socket_node_input(nt, socket):
                        return next((l.from_node for l in nt.links if l.to_socket == socket), None)
                    input_node = socket_node_input(ntree, socket)
                    sub_xml_node = ET.SubElement( xml_node , 'Property' , name=socket.name , node=input_node.bl_idname)

                    export_props(input_node,sub_xml_node)
                else:
                    ET.SubElement( xml_node , 'Property' , name=socket.name , value=socket.export_socket_value() )

        export_props(output_node, mat_node)

