#
#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2023 by Jiayin Cao - All rights reserved.
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

print('''
Available commands

    * final
        Build final version.
    * release
        Build release version.
    * debug
        Build debug version.
    * update
        Sync the latest code from Github
    * update_dep
        Update dependency files.
    * clean
       Clean up all generated file, including the binaries.
       But dependency will still exist.
    * clean_dep
       Clean the dependency.
    * help
        Print this help message.
    * about
        Introduction about SORT and myself.
    * dep_info
        Introduction about the third party libraries used in SORT.

Convenience targets

    * final
        The final version that has no debugging information. This is the
        best option for practical rendering.
    * release
        The relatively performant build version that provides reasonable
        information, like statistics and log. However, there is no debug
        information.
    * debug
        Standard debugging version that has everything, which is only for
        debugging purposes.
''')