:argv_loop
if NOT "%1" == "" (
	if "%1" == "help" (
		set SHOW_HELP=1
		goto EOF
	) else if "%1" == "about" (
		set SHOW_ABOUT=1
		goto EOF
	) else if "%1" == "dep_info" (
		set SHOW_DEP_INFO=1
		goto EOF
	)
)
:EOF
exit /b 0
:ERR
exit /b 1