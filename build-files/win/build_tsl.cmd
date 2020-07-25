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

:: enter the directory of dependencies
call cd dependencies

:: Tiny shading language is another personal project of mine, it is used as the shading language in SORT.
call git clone https://github.com/JiayinCao/Tiny-Shading-Language.git

:: enter the directory
call cd Tiny-Shading-Language

:: trigger an installation of the library
call make update_dep
call make install

:: copy the library to the correct place
call xcopy .\tsl ..\tsl /E /C /I

:: clean up
call cd ..
call rm -rf Tiny-Shading-Language
call cd ..