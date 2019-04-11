#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
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

# This file provides interfaces for parsing parameters defined in OSL shaders.
# This may not be a perfect solution comparing with calling osl APIs, but it is simple enough
# and should be good enough for SORT renderer.

import bpy
import nodeitems_utils
from . import properties
from .. import base

# remove all comments from source code
def removeComments(source):
    res, buffer, block_comment_open = '', '', False
    lines = source.split( '\n' )
    for line in lines:
        i = 0
        while i < len(line):
            char = line[i]
            if char == '/' and (i + 1) < len(line) and line[i + 1] == '/' and not block_comment_open:
                i = len(line)
            elif char == '/' and (i + 1) < len(line) and line[i + 1] == '*' and not block_comment_open:
                block_comment_open = True
                i += 1
            elif char == '*' and (i + 1) < len(line) and line[i + 1] == '/' and block_comment_open:
                block_comment_open = False
                i += 1
            elif not block_comment_open:
                buffer += char
            i += 1
        if buffer and not block_comment_open:
            res += buffer + '\n'
            buffer = ''
    return res

# find the first shader keyword
def findShaderKeyword(source):
    # separate parameters before analysing each parameter for simplicity
    i = -1
    while True:
        # find keyword 'shader'
        i = source.find( 'shader' , i + 1 )
        # this is the end of the shader, it is most likely a broken one
        if i + 6 == len(source):
            return -1
        # there is no such a keyword at all
        if i == -1:
            return -1
        # check if the shader keyword is wrapped by white space
        if ( i == 0 or source[i-1].isspace() ) and source[i+6].isspace():
            break
    return i

# find the function name
def extractFunctionName(source,i):
    function_name = ''
    # skip the white spaces
    while i < len(source) and source[i].isspace():
        i += 1
    # load the shader function name
    while i < len(source) and source[i].isalnum() :
        function_name += source[i]
        i += 1
    while i < len(source) and source[i] != '(':
        i += 1
    return ( function_name , i + 1 )

# extract parameter list
def extractParameters(source,i):
    parameters = []
    while i < len(source):
        parameter_data = ''
        meta_data = False
        in_property = False
        last_char_is_space = True
        while i < len(source) and ( meta_data or in_property or ( source[i] is not ',' and source[i] is not ')' ) ):
            if source[i].isspace() is False:
                parameter_data += source[i]
                last_char_is_space = False
            elif last_char_is_space is False:
                parameter_data += ' '
                last_char_is_space = True

            if source[i] == '[':
                assert( i + 1 < len(source) and source[i+1] == '[' )
                i += 1
                meta_data = True
            if source[i] == ']':
                assert( i + 1 < len(source) and source[i+1] == ']' )
                i += 1
                meta_data = False
            if source[i] == '(':
                in_property = True
            if source[i] == ')':
                in_property = False
            i += 1
        parameters.append( parameter_data )

        if i < len(source) and source[i] is ')':
            break
        i += 1

    return parameters

# parse one single parameter
def parseOneParameter( source ):
    def getParameterNameAndValue( source , i ):
        name = ''
        while i < len(source) and source[i] != '=' and source[i].isspace() is False:
            name += source[i]
            i += 1

        # get to the next token, it could be a '=' sign, a comma, meaning no value assigned
        while i < len(source) and source[i] != '=' and source[i] != ',':
            i += 1

        value = []
        if i < len(source) and source[i] == '=':
            i += 1
            while i < len(source) and source[i].isspace():
                i += 1

            if source.startswith( 'color' , i ) or source.startswith( 'normal' , i ) or source.startswith( 'vector' , i ):
                while i < len(source) and source[i] != '(':
                    i += 1
                assert( i < len(source) and source[i] == '(' )
                i += 1
                j = i
                while( j < len( source ) and source[j] != ')' ):
                    j += 1
                assert( j < len(source) and source[j] == ')' )
                tokens = source[i:j].split(',')
                if len(tokens) == 3:
                    value = ( float(tokens[0]) , float(tokens[1]) , float(tokens[2]) )
                elif len(tokens) == 1:
                    value = ( float(tokens[0]) , float(tokens[0]) , float(tokens[0]) )
            elif i < len(source) and ( source[i].isdigit() or source[i] == '.' ):
                j = i
                while j < len(source) and ( source[i].isdigit() or source[i] == '.' ):
                    j += 1
                value = float( source[i:j] )
            elif i < len(source) and source[i] == '\"':
                # it must be string type
                j = i
                while j < len(source) and ( source[i].isdigit() or source[i] == '\"' ):
                    j += 1
                value = source[i:j]
        return ( name , value )

    if source.startswith( 'color' ):
        return ( 'SORTNodeSocketColor' , getParameterNameAndValue( source , len('color ' ) ) )
    if source.startswith( 'normal' ):
        return ( 'SORTNodeSocketNormal' , getParameterNameAndValue( source , len('normal ' ) ) )
    if source.startswith( 'closure color' ):
        return ( 'SORTNodeSocketBxdf' , getParameterNameAndValue( source , len('closure color ' ) ) )
    if source.startswith( 'float' ):
        return ( 'SORTNodeSocketFloat' , getParameterNameAndValue( source , len('float ' ) ) )
    if source.startswith( 'string' ):
        return ( 'SORTNodePropertyPath' , getParameterNameAndValue( source , len('string ' ) ) )
    return ( 'SORTNodeSocketLargeFloat' , 'Default' )

# extract meta data in parameters
def extractMetaData( source ):
    ret = []
    meta_data_parameters = extractParameters( source , 0 )
    for meta_data in meta_data_parameters:
        ret.append(parseOneParameter( meta_data ))
    return ret

# extract osl parameter
def extractParameter( source ):
    meta_data_begin = source.find( '[' )
    meta_data_end = source.find( ']' )
    metadata = []
    if meta_data_begin >= 0 and meta_data_end >= 0:
        # extract meta data
        metadata = extractMetaData( source[meta_data_begin+1:meta_data_end] )

        # trim the meta data string
        source = source[:meta_data_begin] + source[meta_data_end+1:]

    param = parseOneParameter( source )
    return (param[0] , param[1] , metadata)

# parse osl parameters
def parse_osl_params(osl_shader_source):
    # remove comment first to get rid of some noise signal
    clean_shader = removeComments( osl_shader_source )

    # find the index where the shader decleration starts
    i = findShaderKeyword(clean_shader)
    function_name, i = extractFunctionName(clean_shader, i + 6)
    parameters = extractParameters(clean_shader, i)

    # parse the output parameters
    inputs_list = []
    outputs_list = []
    print( clean_shader )
    for parameter in parameters:
        if parameter.startswith( 'output' ):
            data = extractParameter( parameter[len('output '):] )
            outputs_list.append( data )
            print(data)
        else:
            data = extractParameter( parameter )
            inputs_list.append( data )
            print(data)

    return ( inputs_list , outputs_list )