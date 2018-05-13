import os
import mathutils
import bpy
from .. import utility

def getCamera(scene):
    camera = next(cam for cam in scene.objects if cam.type == 'CAMERA' )
    if camera is None:
        print("Camera not found.")
        return
    return camera

# get edited file name
def getEditedFileName():
    pbrt_file_name = os.path.splitext(os.path.basename(bpy.data.filepath))[0]
    if not pbrt_file_name:
        pbrt_file_name = 'default_scene'
    return pbrt_file_name

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