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

:EOF