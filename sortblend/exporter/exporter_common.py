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

import os
import mathutils
import bpy

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

# list all objects in the scene
def renderable_objects(scene):
    def is_renderable(scene, ob):
        # whether the object is hidden
        def is_visible_layer(scene, ob):
            for i in range(len(scene.layers)):
                if scene.layers[i] == True and ob.layers[i] == True:
                    return True
            return False
        return (is_visible_layer(scene, ob) and not ob.hide_render)
    return [ob for ob in scene.objects if is_renderable(scene, ob)]

# matrix to string
def matrixtostr(matrix):
    return '%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f '%(matrix[0][0],matrix[0][1],matrix[0][2],matrix[0][3],matrix[1][0],matrix[1][1],matrix[1][2],matrix[1][3],matrix[2][0],matrix[2][1],matrix[2][2],matrix[2][3],matrix[3][0],matrix[3][1],matrix[3][2],matrix[3][3])

def vec3tostr(vec):
    return "%f %f %f"%(vec[0],vec[1],vec[2])

def getPreference():
    return bpy.context.user_preferences.addons['sortblend'].preferences

# get the list of material for the whole scene
def getMaterialList( scene ):
    exported_materials = []
    all_nodes = renderable_objects(scene)
    for ob in all_nodes:
        if ob.type == 'MESH':
            for material in ob.data.materials[:]:
                # make sure it is a SORT material
                if material and material.sort_material and material.sort_material.sortnodetree:
                    # skip if the material is already exported
                    if exported_materials.count( material ) != 0:
                        continue
                    exported_materials.append( material )
    return exported_materials

# get the list of lights in the scene
def getLightList( scene ):
    exported_lights = []
    all_nodes = renderable_objects(scene)
    for ob in all_nodes:
        if ob.type == 'LAMP':
            exported_lights.append( ob )
    return exported_lights

# get the list of lights in the scene
def getMeshList( scene ):
    exported_meshes = []
    all_nodes = renderable_objects(scene)
    for ob in all_nodes:
        if ob.type == 'MESH':
            exported_meshes.append( ob )
    return exported_meshes