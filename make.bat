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
	powershell .\build-files\win\getdep.ps1
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
	cd ..
)

if "%BUILD_DEBUG%" == "1" (
	powershell New-Item -Force -ItemType directory -Path proj_debug
	cd proj_debug
	cmake -A x64 ..
	msbuild /p:Configuration=Debug SORT.sln
	cd ..
)

:EOF