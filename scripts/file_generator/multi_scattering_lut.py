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

import os
from . import common
from math import sin, cos, sqrt, acos, atan

PI = 3.1415926
INV_PI = 1.0 / PI
TWO_PI = 2.0 * PI
INV_TWOPI = 1.0 / TWO_PI

# utility helper functions
def sqr(x):
    return x * x

def vectorFromEular(theta, phi):
    wi_x = sin(phi) * sin(theta)
    wi_y = cos(theta)
    wi_z = cos(phi) * sin(theta)
    return (wi_x, wi_y, wi_z)

def middleVector(wo, wi):
    wh_x = 0.5 * ( wo[0] + wi[0] )
    wh_y = 0.5 * ( wo[1] + wi[1] )
    wh_z = 0.5 * ( wo[2] * wi[2] )
    denom = sqrt( sqr(wh_x) + sqr(wh_y) + sqr(wh_z) )
    return (wh_x / denom, wh_y / denom, wh_z / denom)

def dot(wo, wi):
    return wo[0] * wi[0] + wo[1] * wi[1] + wo[2] * wi[2]

# Following implementation has to match the implementation in SORT
def sample_microfacet_ggx(roughness, n_dot_v, r0, r1):
    # Sampling Microfacet Brdf
    # https://agraphicsguynotes.com/posts/sample_microfacet_brdf/
    alpha = sqr(roughness)
    theta = atan(alpha * sqrt(r0 / (1.0 - r0)))
    phi = TWO_PI * r1
    wh = vectorFromEular(theta, phi)

    # evaluate the pdf of the sampling
    # note, the ggx evaluation can actually be skipped since it appears in both nominator and denominator
    # the ggx term is ignored here
    wi = vectorFromEular(acos(n_dot_v), 0.0)
    alpha_sqr = sqr(alpha)
    i_dot_h = dot(wh, wi)
    n_dot_h = wh[1]
    pdf = 1.0 * abs(n_dot_h) / (4.0 * i_dot_h)

    # reflect the outgoing direction
    wo_x = 2.0 * wh[0] * i_dot_h - wi[0]
    wo_y = 2.0 * wh[1] * i_dot_h - wi[1]
    wo_z = 2.0 * wh[2] * i_dot_h - wi[2]

    # for sampled rays below the hemisphere, simply ignore it
    if wo_y <= 0.0:
        return (wo_x, wo_y, wo_z), i_dot_h, 0.0

    # Smith term for GGX
    # ["Geometrical shadowing of a random rough surface", Smith 1967]
    n_dot_l = wo_y
    n_dot_v_sqr = sqr(n_dot_v)
    n_dot_l_sqr = sqr(n_dot_l)
    vis_smith_v = n_dot_v + sqrt( n_dot_v_sqr * ( 1.0 - alpha_sqr ) + alpha_sqr )
    vis_smith_l = n_dot_l + sqrt( n_dot_l_sqr * ( 1.0 - alpha_sqr ) + alpha_sqr )

    # fresnel is ignored for now
    # note, the ggx term actually gets canceled since it appears in both nominator and denominator
    brdf = 1.0 / (vis_smith_l * vis_smith_v)

    return (wo_x, wo_y, wo_z), i_dot_h, brdf / pdf

# Hammersley Points on the Hemisphere
# http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
def radicalInverse_VdC(bits):
     bits = (bits << 16) | (bits >> 16);
     bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1)
     bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2)
     bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4)
     bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8)
     return float(bits) * 2.3283064365386963e-10; # / 0x100000000
def hammersley2d(i, N):
    return float(i)/float(N), radicalInverse_VdC(i)

# Revisiting Physically Based Shading at Imageworks
# http://www.aconty.com/pdf/s2017_pbs_imageworks_slides.pdf
def generate():
    # target file name
    target_filename = "multi_scattering_lut.h"

    # if the target file is already generated, avoid generating it again
    if os.path.isfile(target_filename):
        return

    # output some log
    print('Generating file ' + target_filename + '.')

    # open the file to be written
    f = open(target_filename, "w")

    # generate header and warnings
    common.generate_header(f)

    # make sure this file is only compiled once
    f.write('#pragma once\n\n')

    # helper function to evaluate monte carlo sample
    def evaluate_microfacet(sample_cnt, monte_carlo_sample_cnt, has_fresnel, f0):
        for i in range(sample_cnt):
            roughness = (float)(i) * ( 1.0 / float(sample_cnt - 1) )

            # zero roughness always causes trouble
            roughness = max( 0.001, roughness )

            for j in range(sample_cnt):
                n_dot_v = (float)(j) * ( 1.0 / float(sample_cnt - 1) )

                # we don't allow perfect perpidencular viewing angle
                n_dot_v = max( 0.0001 , n_dot_v )

                E = 0.0

                # I need to make it multi-threaded later
                for k in range(monte_carlo_sample_cnt):
                    # the random number generator has to fullfill two requirements
                    #  - it can't be random every time it gets called, it has to generate exactly the same squence of numbers
                    #  - it'd better to cover the whole domain for better convergency
                    r0, r1 = hammersley2d(k, monte_carlo_sample_cnt)

                    # importance sampling microfacet model
                    wi, i_dot_h, brdf = sample_microfacet_ggx(roughness, n_dot_v, r0, r1)

                    # add fresnel term if needed
                    if has_fresnel:
                        x = 1.0 - i_dot_h
                        x2 = x * x
                        x4 = x2 * x2
                        fresnel = f0 + x * x4 * ( 1.0 - f0 )
                        brdf = brdf * fresnel

                    # accumulate in the approximation if there is positive value
                    E = E + brdf * wi[1]

                E = E / float(monte_carlo_sample_cnt)

                # clamp the value, this should not be needed if there is no precision issue
                E = max( 0.0, min( 1.0, E ) )

                yield E, i, j

    # helper function to generate the lut, this code will be reused a few of times
    def generate_lut(f, sample_cnt, monte_carlo_sample_cnt, ns, with_fresnel):
        f.write('namespace ' + ns + '{\n')
        f.write('constexpr int sample_cnt = {};\n\n'.format(sample_cnt))
        f.write('// Hemispherical-directional reflectance\n')
        f.write('static float g_ms_E[] = {')

        fresnel_cnt = 1 if with_fresnel is False else sample_cnt

        E_data = [[[0 for x in range(sample_cnt)] for y in range(sample_cnt)] for z in range(fresnel_cnt)]

        # evaluate all samples
        for fidx in range(fresnel_cnt):
            f0 = (float)(fidx) * ( 1.0 / float(fresnel_cnt - 1) ) if with_fresnel else 0.0
            for E, i, j in evaluate_microfacet(sample_cnt, monte_carlo_sample_cnt, with_fresnel, f0):
                # Keep track of the data for later usage
                E_data[fidx][i][j] = E

                if j % 8 == 0:
                    f.write('\n    ')

                f.write("{:1.4f}f".format(E))
                f.write(', ')

        f.write('\n};\n')
        f.write('static_assert(sizeof(g_ms_E) == {} * sizeof(float));\n\n'.format(sample_cnt * sample_cnt * fresnel_cnt))

        f.write('// Hemispherical-hemispherical reflectance\n')
        f.write('static float g_ms_Eavg[] = {')

        # this is not exactly Monte Carlo though
        for fidx in range(fresnel_cnt):
            for i in range(sample_cnt):
                E_avg = 0.0
                for k in range(sample_cnt):
                    n_dot_v = (float)(k) * ( 1.0 / float(sample_cnt - 1) )
                    E_avg = E_avg + E_data[fidx][i][k] * n_dot_v
                E_avg = E_avg * 2.0 / float(sample_cnt)

                if i % 8 == 0:
                    f.write('\n    ')
                f.write("{:1.4f}f".format(E_avg))
                f.write(', ')

        f.write('\n};\n')
        f.write('static_assert(sizeof(g_ms_Eavg) == {} * sizeof(float));\n'.format(sample_cnt * fresnel_cnt))

        f.write('}\n\n')

    generate_lut(f, 32, 64, 'multi_scattering_ggs_no_fresnel', False)
    generate_lut(f, 16, 64, 'multi_scattering_ggs_fresnel', True)

    # close the file handle
    f.close()
