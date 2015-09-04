import bpy
import os
import shutil
import numpy as np
from . import preference
from . import common
from . import nodes
from . import utility
import xml.etree.cElementTree as ET

# export blender information
def export_blender(scene):
    # create immediate file path
    create_path(scene)
    # export sort file
    export_sort_file(scene)
    # export scene
    export_scene(scene)
    # export material
    export_material()

# clear old data and create new path
def create_path(scene):
    # get immediate directory
    output_dir = preference.get_immediate_dir()
    output_res_dir = preference.get_immediate_res_dir();
    # clear the old directory
    shutil.rmtree(output_dir, ignore_errors=True)
    # create one if there is no such directory
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)
    if not os.path.exists(output_res_dir):
        os.mkdir(output_res_dir)

# get camera data
def lookAt(camera):
    # it seems that the matrix return here is the inverse of view matrix.
    matrix = camera.matrix_world.copy()
    # get the transpose matrix
    matrix = matrix.transposed()
    pos = matrix[3]             # get eye position
    forwards = -matrix[2]       # get forward direction
    target = (pos + forwards)   # get target
    up = matrix[1]              # get up direction
    return (pos, target, up)

# open sort file
def export_sort_file(scene):
    # create root node
    root = ET.Element("Root")
    # the scene node
    ET.SubElement(root, 'Scene', value='blender_intermediate/blender.xml')
    # the integrator node
    integrator_type = bpy.data.scenes[0].integrator_type_prop
    ET.SubElement(root, 'Integrator', type=integrator_type)
    # image size
    xres = bpy.data.scenes[0].render.resolution_x * bpy.data.scenes[0].render.resolution_percentage / 100
    yres = bpy.data.scenes[0].render.resolution_y * bpy.data.scenes[0].render.resolution_percentage / 100
    ET.SubElement(root, 'RenderTargetSize', w='%d'%xres, h='%d'%yres )
    # output file name
    ET.SubElement(root, 'OutputFile', name='blender_intermediate/blender_generated.exr')
    # sampler type
    sampler_type = bpy.data.scenes[0].sampler_type_prop
    sampler_count = bpy.data.scenes[0].sampler_count_prop
    ET.SubElement(root, 'Sampler', type=sampler_type, round='%s'%sampler_count)
    # camera node
    camera = next(cam for cam in scene.objects if cam.type == 'CAMERA' )
    if camera is None:
        print("Camera not found.")
        return
    pos, target, up = lookAt(camera)
    camera_node = ET.SubElement(root, 'Camera', type='perspective')
    ET.SubElement( camera_node , "Property" , name="eye" , value=utility.vec3tostr(pos))
    ET.SubElement( camera_node , "Property" , name="up" , value=utility.vec3tostr(up))
    ET.SubElement( camera_node , "Property" , name="target" , value=utility.vec3tostr(target))
    ET.SubElement( camera_node , "Property" , name="len" , value="0")
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
    aspect_ratio_x = bpy.data.scenes["Scene"].render.pixel_aspect_x
    aspect_ratio_y = bpy.data.scenes["Scene"].render.pixel_aspect_y
    ET.SubElement( camera_node , "Property" , name="aspect" , value="%s %s"%(aspect_ratio_x,aspect_ratio_y))
    fov_angle = bpy.data.cameras[0].angle
    ET.SubElement( camera_node , "Property" , name="fov" , value= "%s"%fov_angle)
    camera_shift_x = bpy.data.cameras[0].shift_x
    camera_shift_y = bpy.data.cameras[0].shift_y
    ET.SubElement( camera_node , "Property" , name="shift" , value="%s %s"%(camera_shift_x,camera_shift_y))
    # output thread num
    thread_num = bpy.data.scenes[0].thread_num_prop
    ET.SubElement( root , 'ThreadNum', name='%s'%thread_num)
    # output the xml
    output_sort_file = preference.get_immediate_dir() + 'blender_exported.xml'
    tree = ET.ElementTree(root)
    tree.write(output_sort_file)

# export scene
def export_scene(scene):
    # create root node
    root = ET.Element("Root")
    # resource path node
    ET.SubElement( root , 'Resource', path="./blender_intermediate/res/")
    # acceleration structure
    accelerator_type = bpy.data.scenes[0].accelerator_type_prop
    ET.SubElement( root , 'Accel', type=accelerator_type)  # to be exposed through GUI
    for ob in renderable_objects(scene):
        if ob.type == 'MESH':
            model_node = ET.SubElement( root , 'Model' , filename=ob.name + '.obj', name = ob.name )
            transform_node = ET.SubElement( model_node , 'Transform' )
            ET.SubElement( transform_node , 'Matrix' , value = 'm '+ utility.matrixtostr(ob.matrix_world) )
            # output the mesh to file
            export_mesh(ob,scene)
        elif ob.type == 'LAMP':
            lamp = bpy.data.lamps['Lamp']
            if lamp.type == 'SUN':
                light_node = ET.SubElement( root , 'Light' , type='distant')
                light_spectrum = np.array(lamp.color[:])
                light_spectrum *= lamp.energy
                light_dir = ob.matrix_world.col[2] * -1.0
                ET.SubElement( light_node , 'Property' , name='intensity' , value=utility.vec3tostr(light_spectrum))
                ET.SubElement( light_node , 'Property' , name='dir' ,value=utility.vec3tostr(light_dir))
            elif lamp.type == 'POINT':
                light_node = ET.SubElement( root , 'Light' , type='point')
                light_spectrum = np.array(lamp.color[:])
                light_spectrum *= lamp.energy
                light_position = ob.matrix_world.col[3]
                ET.SubElement( light_node , 'Property' , name='intensity' , value=utility.vec3tostr(light_spectrum))
                ET.SubElement( light_node , 'Property' , name='pos' ,value=utility.vec3tostr(light_position))
            elif lamp.type == 'SPOT':
                light_node = ET.SubElement( root , 'Light' , type='spot')
                light_spectrum = np.array(lamp.color[:])
                light_spectrum *= lamp.energy
                light_dir = ob.matrix_world.col[2] * -1.0
                light_position = ob.matrix_world.col[3]
                ET.SubElement( light_node , 'Property' , name='intensity' , value=utility.vec3tostr(light_spectrum))
                ET.SubElement( light_node , 'Property' , name='dir' ,value=utility.vec3tostr(light_dir))
                ET.SubElement( light_node , 'Property' , name='falloff_start' ,value='1.0')
                ET.SubElement( light_node , 'Property' , name='range' ,value='1.0')
                ET.SubElement( light_node , 'Property' , name='pos' ,value=utility.vec3tostr(light_position))
            elif lamp.type == 'AREA':
                light_node = ET.SubElement( root , 'Light' , type='area')
                light_spectrum = np.array(lamp.color[:])
                light_spectrum *= lamp.energy
                light_dir = ob.matrix_world.col[2] * -1.0
                light_position = ob.matrix_world.col[3]
                ET.SubElement( light_node , 'Property' , name='shape' ,value='square')
                ET.SubElement( light_node , 'Property' , name='intensity' , value=utility.vec3tostr(light_spectrum))
                ET.SubElement( light_node , 'Property' , name='dir' ,value=utility.vec3tostr(light_dir))
                ET.SubElement( light_node , 'Property' , name='pos' ,value=utility.vec3tostr(light_position))
                ET.SubElement( light_node , 'Property' , name='radius' ,value='10')

    # output the xml
    output_scene_file = preference.get_immediate_dir() + 'blender.xml'
    tree = ET.ElementTree(root)
    tree.write(output_scene_file)

def name_compat(name):
    if name is None:
        return 'None'
    else:
        return name.replace(' ', '_')

mtl_dict = {}
mtl_rev_dict = {}

# export mesh file
def export_mesh(obj,scene):
    output_path = preference.get_immediate_res_dir() + obj.name + '.obj'

    # the mesh object
    mesh = obj.data

    # face index pairs
    face_index_pairs = [(face, index) for index, face in enumerate(mesh.polygons)]

    # generate normal data
    mesh.calc_normals_split()
    with open(output_path, 'w') as file:
        file.write("# OBJ file\n")

        # all materials are exported in blender_material.xml
        file.write("mtllib ../blender_material.xml\n")

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
            file.write("v %.4f %.4f %.4f\n" % v.co[:])

        # output normal
        no_key = no_val = None
        normals_to_idx = {}
        no_get = normals_to_idx.get
        no_unique_count = 0
        loops_to_normals = [0] * len(mesh.loops)
        for f, f_index in face_index_pairs:
            for l_idx in f.loop_indices:
                no_key = utility.veckey3d(mesh.loops[l_idx].normal)
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
            #if f_smooth and smooth_groups:
            #    f_smooth = smooth_groups[f_index]
            f_mat = min(f.material_index, len(materials) - 1)

            #if faceuv:
            #    tface = uv_texture[f_index]
            #    f_image = tface.image

            # MAKE KEY
            #if faceuv and f_image:  # Object is always true.
            #    key = material_names[f_mat], f_image.name
            #else:
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
            for vi, v, li in f_v:
                file.write(" %d//%d" % (v.index+1, loops_to_normals[li]+1))
            file.write("\n")

def export_material():
    # create root node
    root = ET.Element("Root")

    for material in bpy.data.materials:
        if material and material.sort_material and material.sort_material.sortnodetree:
            ntree = bpy.data.node_groups[material.sort_material.sortnodetree]
            output_node = nodes.find_node(material, common.sort_node_output_bl_name)
            if output_node is None:
                continue

            # material node
            mat_node = ET.SubElement( root , 'Material', name=material.name )

            def draw_props(mat_node , xml_node):
                #for prop in mat_node.prop_inputs:
                #    print('Property:'+prop.output_default_value_to_str())
                #    ET.SubElement( xml_node , 'Property' , name=prop.name , type='value', value=prop.output_default_value_to_str() )
                mat_node.export_prop(xml_node)

                inputs = mat_node.inputs
                for socket in inputs:
                    if socket.is_linked:
                        input_node = nodes.socket_node_input(ntree, socket)
                        sub_xml_node = ET.SubElement( xml_node , 'Property' , name=socket.name , type='node', node=input_node.bl_idname)
                        draw_props(input_node,sub_xml_node)
                    else:
                        ET.SubElement( xml_node , 'Property' , name=socket.name , type='value', value=socket.output_default_value_to_str() )

            draw_props(output_node, mat_node)

    # output the xml
    output_material_file = preference.get_immediate_dir() + 'blender_material.xml'
    tree = ET.ElementTree(root)
    tree.write(output_material_file)

# whether the object is hidden
def is_visible_layer(scene, ob):
    for i in range(len(scene.layers)):
        if scene.layers[i] == True and ob.layers[i] == True:
            return True
    return False

# whether the object is renderable
def is_renderable(scene, ob):
    return (is_visible_layer(scene, ob) and not ob.hide_render)

# list all objects in the scene
def renderable_objects(scene):
    return [ob for ob in scene.objects if is_renderable(scene, ob)]