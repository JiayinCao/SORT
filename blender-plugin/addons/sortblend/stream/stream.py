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

import bpy
import struct

class Stream():
    def __init__(self):
        pass

    # Serialize data
    def serialize(self,data):
        pass

# File stream will serialize data into a file.
class FileStream(Stream):
    # Open a file by default
    def __init__(self,filename):
        self.file = open( filename , 'wb' )

    # Make sure we close the file
    def __del__(self):
        self.file.flush()
        self.file.close()

    def flush(self):
        self.file.flush()

    # Serialize data
    def serialize(self,data):
        def serialize_type(data):
            if type(data).__name__ == 'float' or type(data).__name__ == 'float64':
                self.file.write(struct.pack( 'f' , data ) )
            elif type(data).__name__ == 'int':
                self.file.write(struct.pack( 'I' , data ))
            elif type(data).__name__ == 'bool':
                self.file.write(struct.pack( '?' , data ) )

        if type(data).__name__ == 'bytes' or type(data).__name__ == 'bytearray':
            self.file.write(data)
        elif type(data).__name__ == 'str' :
            self.file.write(data.encode('ascii'))
            end = 0
            self.file.write(end.to_bytes(1, byteorder='little'))
        elif type(data).__name__ == 'tuple':
            for d in data:
                serialize_type(d)
        else:
            serialize_type(data)
        self.file.flush()
