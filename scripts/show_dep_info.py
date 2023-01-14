#
#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.
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
#

print(
'''
Following are dependencies used in SORT renderer

   * Tiny Shading Language
       Tiny shading language is another project of mine. Its sole purpose is
       to support programmable shader in SORT. For more detail, please check
       out https://jiayincao.github.io/Tiny-Shading-Language/
   * Easy Profiler [Optional]
       Easy profiler is currently used as main profiling tool. It is more
       of a higher level profiling tool for tasks, instead of low level
       profiling tool.
   * stb_image
       Stb_image is used to read textures from file system. It is already
       included in the source code in the thirdparty folder.
   * Google Test Framework
       SORT uses Google test as its framework for unit testing. This is
       also included in the thirdparty folder in SORT source code.
   * Tiny Exr
       Tiny Exr is a open source library for reading and writting exr files.
       Like the other two libraries, the source code is also included in
       'thirdpard' folder.

''')