import bpy
from bpy.props import PointerProperty, StringProperty, BoolProperty, \
EnumProperty, IntProperty, FloatProperty, FloatVectorProperty, \
CollectionProperty

# basic information in sort material
class SORTMaterial(bpy.types.PropertyGroup):
    sortnodetree = StringProperty(name="Nodetree",default='')
    use_sort_nodes = BoolProperty(name="Nodetree",default=False)

def register():
    bpy.types.Material.sort_material = PointerProperty(type=SORTMaterial, name="SORT Material Settings")

def unregister():
    del bpy.types.Material.sort_material