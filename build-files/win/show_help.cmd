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
