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

@echo off
set SORT_DIR=%~dp0

rem reset all variables first
call "%SORT_DIR%\build-files\win\reset_variables.cmd"

rem parse arguments
call "%SORT_DIR%\build-files\win\parse_arguments.cmd" %*
if errorlevel 1 goto EOF

if "%SHOW_HELP%" == "1" (
	call "%SORT_DIR%\build-files\win\show_help.cmd"
	goto EOF
)

if "%SHOW_ABOUT%" == "1" (
	call "%SORT_DIR%\build-files\win\show_about.cmd"
	goto EOF
)

if "%SHOW_DEP_INFO%" == "1" (
	call "%SORT_DIR%\build-files\win\show_dep_info.cmd"
	goto EOF
)

if "%GIT_UPDATE%" == "1" (
	echo Syncing source code from Github
	git pull
	goto EOF
)

if "%UPDATE_DEP%" == "1" (
	echo Syncing dependencies
	py .\scripts\get_dep.py
	goto EOF
)

if "%FORCE_UPDATE_DEP%" == "1" (
	echo Syncing dependencies
	py .\scripts\get_dep.py TRUE
	goto EOF
)

if "%CLEAN_DEP%" == "1" (
	echo Cleaning all dependency files
	powershell Remove-Item -path ./dependencies -recurse -ErrorAction Ignore
	goto EOF
)

if "%CLEAN%" == "1" (
	echo Cleaning all generated file
	powershell Remove-Item -path ./bin -recurse -ErrorAction Ignore
	powershell Remove-Item -path ./_out -recurse -ErrorAction Ignore
	powershell Remove-Item -path ./proj_release -recurse -ErrorAction Ignore
	powershell Remove-Item -path ./proj_debug -recurse -ErrorAction Ignore
	goto EOF
)

if "%BUILD_RELEASE%" == "1" (
	powershell New-Item -Force -ItemType directory -Path proj_release
	cd proj_release
	cmake -A x64 ..
	msbuild /p:Configuration=Release SORT.sln

	:: catch msbuild error
	if ERRORLEVEL 1 ( 
		goto BUILD_ERR
	)

	cd ..
)

if "%BUILD_RELWITHDEBINFO%" == "1" (
	powershell New-Item -Force -ItemType directory -Path proj_relwithdebinfo
	cd proj_relwithdebinfo
	cmake -A x64 ..
	msbuild /p:Configuration=RelWithDebInfo SORT.sln

	:: catch msbuild error
	if ERRORLEVEL 1 ( 
		goto BUILD_ERR
	)

	cd ..
)

if "%BUILD_DEBUG%" == "1" (
	powershell New-Item -Force -ItemType directory -Path proj_debug
	cd proj_debug
	cmake -A x64 ..
	msbuild /p:Configuration=Debug SORT.sln

	:: catch msbuild error
	if ERRORLEVEL 1 ( 
		goto BUILD_ERR
	)

	cd ..
)

if "%REGISTER_SYS_ENV%" == "1" (
	echo Register environment variables
	echo Setting system environment 'SORT_ROOT_DIR' to %cd%
	setx SORT_ROOT_DIR %cd%/ /M
	echo Setting system environment 'SORT_BIN_DIR' to %cd%/bin
	setx SORT_BIN_DIR %cd%/bin/ /M
)

:EOF
exit /b 0
:BUILD_ERR
exit /b 1