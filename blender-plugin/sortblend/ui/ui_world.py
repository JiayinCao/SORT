#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.
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
import bl_ui
from .. import base

preview_collections = {}
@base.register_class
class SORTHDRSky(bpy.types.PropertyGroup):
    def generate_preview(self, context):
        name = self.name + '_' + self.id_data.name
        if name not in preview_collections:
            item = bpy.utils.previews.new()
            item.previews = ()
            item.image_name = ''
            preview_collections[name] = item
        item = preview_collections[name]
        wm = context.window_manager
        enum_items = []
        img = self.hdr_image
        if img:
            new_image_name = img.name
            if item.image_name == new_image_name:
                return item.previews
            else:
                item.image_name = new_image_name
            item.clear()
            thumb = item.load(img.name, bpy.path.abspath(img.filepath), 'IMAGE')
            
            # somehow, it doesn't show the preview without this line
            thumb.image_size[0]

            enum_items = [(img.filepath, img.name, '', thumb.icon_id, 0)]
        item.previews = enum_items
        return item.previews

    hdr_image : bpy.props.PointerProperty(type=bpy.types.Image)
    preview : bpy.props.EnumProperty(items=generate_preview)
    @classmethod
    def register(cls):
        bpy.types.Scene.sort_hdr_sky = bpy.props.PointerProperty(name="SORT HDR Sky", type=cls)
    @classmethod
    def unregister(cls):
        del bpy.types.Scene.sort_hdr_sky

@base.register_class
class HDRSKY_PT_SORTPanel(bpy.types.Panel):
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "world"
    bl_label = 'HDR Sky'

    COMPAT_ENGINES = {'SORT'}
    @classmethod
    def poll(cls, context):
        return context.scene.render.engine in cls.COMPAT_ENGINES

    def draw(self, context):
        self.layout.template_ID(context.scene.sort_hdr_sky, 'hdr_image', open='image.open')
        self.layout.template_icon_view(context.scene.sort_hdr_sky, 'preview', show_labels=True)