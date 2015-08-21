import bpy
import os
import shutil
from . import preference
import xml.etree.cElementTree as ET

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
    output_dir = get_immediate_dir()
    output_res_dir = get_immediate_res_dir();
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
    ET.SubElement(root, 'Integrator', type='whitted')
    # image size
    ET.SubElement(root, 'RenderTargetSize', w='1440', h='900' )
    # output file name
    ET.SubElement(root, 'OutputFile', name='result.bmp')
    # sampler type
    ET.SubElement(root, 'Sampler', type='stratified', round='1')
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
    # output the xml
    output_sort_file = get_immediate_dir() + 'blender_exported.xml'
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
            export_mesh(ob)
        elif ob.type == 'LAMP':
            light_node = ET.SubElement( root , 'Light' , type='distant')    # to be exposed through GUI
            ET.SubElement( light_node , 'Property' , name='intensity' , value="100")
            ET.SubElement( light_node , 'Property' , name='dir' , value="0 -1 0")

    # output the xml
    output_scene_file = get_immediate_dir() + 'blender.xml'
    tree = ET.ElementTree(root)
    tree.write(output_scene_file)

# export mesh file
def export_mesh(obj):
    output_path = get_immediate_res_dir() + obj.name + '.obj'
    mesh = obj.data
    with open(output_path, 'w') as f:
        f.write("# OBJ file\n")
        f.write("g cube\n")
        for v in mesh.vertices:
            f.write("v %.4f %.4f %.4f\n" % v.co[:])
        for p in mesh.polygons:
            f.write("f")
            for i in p.vertices:
                f.write(" %d" % (i + 1))
            f.write("\n")

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

def get_immediate_dir():
    sort_bin_dir = preference.get_sort_path()
    immediate_dir = sort_bin_dir + 'blender_intermediate/'
    return immediate_dir

def get_immediate_res_dir():
    return get_immediate_dir() + 'res/'