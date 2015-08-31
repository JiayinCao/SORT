# utility function
def vec3tostr(vec):
    return "%f %f %f"%(vec[0],vec[1],vec[2])

# matrix to string
def matrix_to_array(matrix):
    return matrix[0][0],matrix[0][1],matrix[0][2],matrix[0][3],matrix[1][0],matrix[1][1],matrix[1][2],matrix[1][3],matrix[2][0],matrix[2][1],matrix[2][2],matrix[2][3],matrix[3][0],matrix[3][1],matrix[3][2],matrix[3][3]

def matrixtostr(matrix):
    return '%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f '%matrix_to_array(matrix)

def veckey3d(v):
    return round(v.x, 4), round(v.y, 4), round(v.z, 4)