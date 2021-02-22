#
#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.
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

import os
from random import seed, random
from math import sin, cos

# Physically Based Shading at DreamWorks Animation
# https://blog.selfshadow.com/publications/s2017-shading-course/dreamworks/s2017_pbs_dreamworks_notes.pdf

def generate(license_header, warning):
    # open the file to be written
    f = open("fabric_lut.h", "w")

    # license header
    f.write(license_header)
    # warning to indicate not to modify the file
    f.write(warning)

    # make sure this file is only compiled once
    f.write('#pragma once\n\n')

    # 256 is generally good enough for our sampling
    lut_table_element_cnt = 256

    # make sure the result is not inconsistent every time it generates
    seed(0)

    # comment indicate what it is
    f.write( '// Physically Based Shading at DreamWorks Animation \n' )
    f.write( '// https://blog.selfshadow.com/publications/s2017-shading-course/dreamworks/s2017_pbs_dreamworks_notes.pdf \n\n' )
    f.write( '// This is the pre-integrated I_o in the page of 14. \n' )

    # generate the lut table
    lut_content = 'static const float g_fabric_lut[] = {'
    for i in range(lut_table_element_cnt):
        if i % 8 == 0:
            lut_content += '\n    '

        N = 10000
        n = i / 255.0 * 30.0

        sum = 0.0
        for k in range(N):
            r = random() * 3.1415926 * 0.5
            sum += pow( 1.0 - sin( r * 0.5 ) , n ) * cos( r )
        
        sum *= 3.1415926 * 2.0 / N
        lut_content += format(sum, '.4f')

        if i != lut_table_element_cnt - 1:
            lut_content += ', '

    lut_content += '};\n\n'

    # write out the lut content
    f.write(lut_content)

    # make sure there is a static assert to verify the length of the array
    f.write('static_assert( ( sizeof( g_fabric_lut ) / sizeof(float) ) == %d , "Incorrect pre-integrated array size." );\n' % lut_table_element_cnt)

    # close the file handle
    f.close()