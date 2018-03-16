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

# get camera data
def lookAt(camera):
    # it seems that the matrix return here is the inverse of view matrix.
    ori_matrix = utility.getGlobalMatrix() * camera.matrix_world.copy()
    # get the transpose matrix
    matrix = ori_matrix.transposed()
    pos = matrix[3]             # get eye position
    forwards = -matrix[2]       # get forward direction

    # get focal distance for DOF effect
    if camera.data.dof_object is not None:
        focal_object = camera.data.dof_object
        fo_mat = utility.getGlobalMatrix() * focal_object.matrix_world
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