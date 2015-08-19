import bpy

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
    archive_obs = []
    for ob in renderable_objects(scene):
        # explicitly set
        print("  "+ob.name)

    return archive_obs