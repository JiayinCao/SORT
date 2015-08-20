import bpy
import os
import shutil
from . import preference
import xml.etree.cElementTree as ET

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

# export scene
def export_scene(scene):
    print(" export_scene")
    export_sort_file(scene)

    #archive_obs = []
    #for ob in renderable_objects(scene):
        # explicitly set
    #    print("  "+ob.name)

    return

# open sort file
def export_sort_file(scene):
    sort_bin_dir = preference.get_sort_path()
    output_dir = sort_bin_dir + 'blender_intermediate/'

    # clear the old directory
    shutil.rmtree(output_dir, ignore_errors=True)

    # create one if there is no such directory
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)

    # create sort file
    output_sort_file = output_dir + 'blender_exported.sort'

    root = ET.Element("Root")

    # the scene node
    ET.SubElement(root, 'Scene', value='blender.scene')

    # the integrator node
    ET.SubElement(root, 'Integrator', type='whitted')

    # image size
    ET.SubElement(root, 'RenderTargetSize', w='1440', h='900' )

    # output file name
    ET.SubElement(root, 'OutputFile', name='result.bmp')

    # sampler type
    ET.SubElement(root, 'Sampler', type='stratified', round='1')

    # camera node
    camera_node = ET.SubElement(root, 'Camera', type='perspective')
    ET.SubElement( camera_node , "Property" , name="eye" , value="")
    ET.SubElement( camera_node , "Property" , name="up" , value="")
    ET.SubElement( camera_node , "Property" , name="target" , value="")
    ET.SubElement( camera_node , "Property" , name="len" , value="")
    ET.SubElement( camera_node , "Property" , name="interaxial" , value="")
    ET.SubElement( camera_node , "Property" , name="width" , value="")
    ET.SubElement( camera_node , "Property" , name="height" , value="")

    tree = ET.ElementTree(root)
    tree.write(output_sort_file)