SRC_PATH = ../src
OBJ_PATH = obj

sort : $(OBJ_PATH) sort.o vector.o point.o normal.o
	g++ -o sort $(OBJ_PATH)/sort.o $(OBJ_PATH)/vector.o $(OBJ_PATH)/point.o $(OBJ_PATH)/normal.o
	#delete the obj path
	rm -rf $(OBJ_PATH)

$(OBJ_PATH):
	mkdir -p $(OBJ_PATH)

normal.o : $(SRC_PATH)/geometry/normal.cpp
	g++ -c $(SRC_PATH)/geometry/normal.cpp -o $(OBJ_PATH)/normal.o

point.o : $(SRC_PATH)/geometry/point.cpp
	g++ -c $(SRC_PATH)/geometry/point.cpp -o $(OBJ_PATH)/point.o

sort.o : $(SRC_PATH)/sort.cpp
	g++ -c $(SRC_PATH)/sort.cpp -o $(OBJ_PATH)/sort.o

vector.o : $(SRC_PATH)/geometry/vector.cpp
	g++ -c $(SRC_PATH)/geometry/vector.cpp -o $(OBJ_PATH)/vector.o
