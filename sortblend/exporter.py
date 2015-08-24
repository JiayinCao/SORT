import bpy
import os
import shutil
from . import preference
import xml.etree.cElementTree as ET
from io_scene_obj import export_obj

# export blender information
def export_blender(scene):
    # create immediate file path
    create_path(scene)
    # export sort file
    export_sort_file(scene)
    # export scene
    export_scene(scene)

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
    ET.SubElement( camera_node , "Property" , name="eye" , value="%f %f %f"%vector_to_array3f(pos))
    ET.SubElement( camera_node , "Property" , name="up" , value="%f %f %f"%vector_to_array3f(up))
    ET.SubElement( camera_node , "Property" , name="target" , value="%f %f %f"%vector_to_array3f(target))
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
    ET.SubElement( camera_node , "Property" , name="sensorsize" , value= "%s %s %f"%(sensor_w,sensor_h, sensor_fit))
    aspect_ratio_x = bpy.data.scenes["Scene"].render.pixel_aspect_x
    aspect_ratio_y = bpy.data.scenes["Scene"].render.pixel_aspect_y
    ET.SubElement( camera_node , "Property" , name="aspect" , value="%s %s"%(aspect_ratio_x,aspect_ratio_y))
    fov_angle = bpy.data.cameras[0].angle
    ET.SubElement( camera_node , "Property" , name="fov" , value= "%s"%fov_angle)
    # output thread num
    thread_num = bpy.data.scenes[0].thread_num_prop
    ET.SubElement( root , 'ThreadNum', name='%s'%thread_num)
    # output the xml
    output_sort_file = preference.get_immediate_dir() + 'blender_exported.xml'
    tree = ET.ElementTree(root)
    tree.write(output_sort_file)

# vector to string
def vector_to_array4f(vector):
    return vector[0],vector[1],vector[2],vector[3]
def vector_to_array3f(vector):
    return vector[0],vector[1],vector[2]

# matrix to string
def matrix_to_array(matrix):
    return matrix[0][0],matrix[0][1],matrix[0][2],matrix[0][3],matrix[1][0],matrix[1][1],matrix[1][2],matrix[1][3],matrix[2][0],matrix[2][1],matrix[2][2],matrix[2][3],matrix[3][0],matrix[3][1],matrix[3][2],matrix[3][3]

# export scene
def export_scene(scene):
    # create root node
    root = ET.Element("Root")
    # resource path node
    ET.SubElement( root , 'Resource', path="./blender_intermediate/res/")
    # acceleration structure
    ET.SubElement( root , 'Accel', type="kd_tree")  # to be exposed through GUI
    for ob in renderable_objects(scene):
        if ob.type == 'MESH':
            model_node = ET.SubElement( root , 'Model' , filename=ob.name + '.obj', name = ob.name )
            transform_node = ET.SubElement( model_node , 'Transform' )
            matrix = ob.matrix_world;
            ET.SubElement( transform_node , 'Matrix' , value = 'm %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f '%matrix_to_array(matrix) )
            # output the mesh to file
            export_mesh(ob,scene)
        elif ob.type == 'LAMP':
            light_node = ET.SubElement( root , 'Light' , type='distant')    # to be exposed through GUI
            ET.SubElement( light_node , 'Property' , name='intensity' , value="10 10 10")
            ET.SubElement( light_node , 'Property' , name='dir' , value="-1 -2 -3")

    # output the xml
    output_scene_file = preference.get_immediate_dir() + 'blender.xml'
    tree = ET.ElementTree(root)
    tree.write(output_scene_file)

def veckey3d(v):
    return round(v.x, 4), round(v.y, 4), round(v.z, 4)

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
        file.write("g cube\n")
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
            f_v = [(vi, me_verts[v_idx], l_idx)
                       for vi, (v_idx, l_idx) in enumerate(zip(f.vertices, f.loop_indices))]
            file.write("f")
            for vi, v, li in f_v:
                file.write(" %d//%d" % (v.index+1, loops_to_normals[li]+1))
            file.write("\n")

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