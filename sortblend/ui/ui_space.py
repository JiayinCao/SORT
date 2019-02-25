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

from .. import base
from bl_ui import properties_data_mesh

base.compatify_class(properties_data_mesh.DATA_PT_normals)
base.compatify_class(properties_data_mesh.DATA_PT_context_mesh)
base.compatify_class(properties_data_mesh.DATA_PT_texture_space)
base.compatify_class(properties_data_mesh.DATA_PT_vertex_groups)
base.compatify_class(properties_data_mesh.DATA_PT_shape_keys)
base.compatify_class(properties_data_mesh.DATA_PT_customdata)
base.compatify_class(properties_data_mesh.DATA_PT_vertex_colors)
base.compatify_class(properties_data_mesh.DATA_PT_uv_texture)