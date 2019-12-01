echo.
echo Available commands
echo:
echo     * final
echo         Build final version.
echo     * release
echo         Build release version.
echo     * debug
echo         Build debug version.
echo     * update
echo         Sync the latest code from Github
echo     * update_dep
echo         Update dependency files.
echo     * clean
echo        Clean up all generated file, including the binaries.
echo        But dependency will still exist.
echo     * clean_dep
echo        Clean the dependency.
echo     * help
echo         Print this help message.
echo     * about
echo         Introduction about SORT and myself.
echo     * dep_info
echo         Introduction about the third party libraries used in SORT.
echo:
echo Convenience targets
echo:
echo     * final       
echo         The final version that has no debugging information. This is the
echo         best option for practical rendering.
echo     * release
echo         The relatively performant build version that provides reasonable 
echo         information, like statistics and log. However, there is no debug
echo         information.
echo     * debug
echo         Standard debugging version that has everything, which is only for
echo         debugging purposes.
echo.