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

REGISTRARS = []
def registrar(register, unregister, name=None):
    global REGISTRARS
    if name is None or not [True for _, _, n in REGISTRARS if n == name]:
        REGISTRARS.append((register, unregister, name))

def register():
    for r, _, _ in REGISTRARS:
        r()

def unregister():
    for _, u, _ in reversed(REGISTRARS):
        u()

def register_class(cls):
    registrar(lambda: bpy.utils.register_class(cls), lambda: bpy.utils.unregister_class(cls), cls.__name__)
    return cls

def compatify_class(cls):
    def reg():
        cls.COMPAT_ENGINES.add('SORT')
    def unreg():
        cls.COMPAT_ENGINES.remove('SORT')
    registrar(reg, unreg, cls.__name__)
    return cls