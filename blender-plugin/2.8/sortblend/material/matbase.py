#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
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
import nodeitems_utils
from .. import base, renderer

class SORTPatternNodeCategory(nodeitems_utils.NodeCategory):
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == SORTShaderNodeTree.bl_idname and renderer.SORTRenderEngine.is_active(context)

def tmp_sort_node_group_items(context):
    if context is None:
        return
    space = context.space_data
    if not space:
        return

    tree = space.edit_tree
    if not tree:
        return

    def group_tools_draw(self, layout, context):
        layout.operator("sort.node_group_make")
        layout.operator("sort.node_group_ungroup")
        layout.separator()

    yield nodeitems_utils.NodeItemCustom(draw=group_tools_draw)

@base.register_class
class SORTShaderNodeTree(bpy.types.NodeTree):
    bl_idname = 'SORTShaderNodeTree'
    bl_label = 'SORT Shader Editor'
    bl_icon = 'MATERIAL'
    node_categories = {}

    # Return a node tree from the context to be used in the editor
    @classmethod
    def get_from_context(cls, context):
        ob = context.active_object
        if ob is not None and ob.active_material is not None:
            mat = ob.active_material
            return mat.sort_material , mat , mat
        return (None, None, None)

    @classmethod
    def register_node(cls,category):
        def registrar(nodecls):
            base.register_class(nodecls)
            d = cls.node_categories.setdefault(category, [])
            d.append(nodecls)
            return nodecls
        return registrar

    @classmethod
    def register(cls):
        bpy.types.Material.sort_material = bpy.props.PointerProperty(type=bpy.types.NodeTree, name='SORT Material Settings')

        # Register all nodes
        cats = []
        for c, l in sorted(cls.node_categories.items()):
            cid = 'SORT_' + c.replace(' ', '').upper()
            items = [nodeitems_utils.NodeItem(nc.__name__) for nc in l]
            cats.append(SORTPatternNodeCategory(cid, c, items=items))
        cats.append(SORTPatternNodeCategory('SORT_LAYOUT', 'Layout', items=[nodeitems_utils.NodeItem('NodeFrame'),nodeitems_utils.NodeItem('NodeReroute')]))
        cats.append(SORTPatternNodeCategory('SORT_Group', 'Group', items=tmp_sort_node_group_items))

        nodeitems_utils.register_node_categories('SHADER_NODES_SORT', cats)

    @classmethod
    def unregister(cls):
        nodeitems_utils.unregister_node_categories('SHADER_NODES_SORT')
        del bpy.types.Material.sort_material