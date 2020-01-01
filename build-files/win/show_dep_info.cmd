::
::    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
::    platform physically based renderer.
::
::    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.
::
::    SORT is a free software written for educational purpose. Anyone can distribute
::    or modify it under the the terms of the GNU General Public License Version 3 as
::    published by the Free Software Foundation. However, there is NO warranty that
::    all components are functional in a perfect manner. Without even the implied
::    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
::    General Public License for more details.
::
::    You should have received a copy of the GNU General Public License along with
::    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
::

echo.
echo Following are dependencies used in SORT renderer
echo:
echo    * Open Shading Language
echo        OSL is what SORT is currently using as shading language. With the
echo        support of OSL, SORT can compile shaders during runtime to allow
echo        artist more flexible support in shader assets.
echo    * Easy Profiler
echo        Easy profiler is currently used as main profiling tool. It is more
echo        of a higher level profiling tool for tasks, instead of low level
echo        profiling tool.
echo    * stb_image
echo        Stb_image is used to read textures from file system. It is already
echo        included in the source code in the thirdparty folder.
echo    * Google Test Framework
echo        SORT uses Google test as its framework for unit testing. This is
echo        also included in the thirdparty folder in SORT source code.
echo    * Tiny Exr
echo        Tiny Exr is a open source library for reading and writting exr files.
echo        Like the other two libraries, the source code is also included in
echo        'thirdpard' folder.
echo.
