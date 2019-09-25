#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.
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

# cooresponding implementation of strid.h in python code.
# The algorithm has to be exactly the same with the one defined in strid.h

# Computation of cyclic redundancy checks
# https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks
def crc32_bitwise( data , len ):
    Polynomial = 0xEDB88320

    crc = 0
    i = 0
    while i < len:
        crc ^= ord(data[i])

        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial)
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial)
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial)
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial)
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial)
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial)
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial)
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial)

        i += 1

    # Technically speaking, this is a signed integer, but since the bits are the same and
    # the value is never used in a mathematic operation, this is not a big problem.
    return ~crc

# convert a string to hashed key
def SID( str ):
    return crc32_bitwise( str , len(str) )
